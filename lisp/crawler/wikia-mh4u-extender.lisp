;;;; wikia-mh4u-extender
;;;; Author: Break Yang (breakds@gmail.com)
;;;; Description: Extends MH4G data with 4U English names.


(in-package #:breakds.monster-avengers.mh4g-crawler)

(defparameter *skill-list-url* "http://monsterhunter.wikia.com/wiki/MH4U:_Armor_Skill_List")
(defparameter *armor-list-url* "http://monsterhunter.wikia.com/wiki/MH4U:_Armor")

(defun get-href (node)
  (second (assoc "href" (get-attributes node)
                 :test #'equal)))

(defun get-trimmed-text (node)
  (loop for child in (get-children node)
     when (stringp child)
     do (let ((text (string-trim-all child)))
          (when (> (length text) 0)
            (return text)))))

(defun get-trimmed-paragraph (node)
  (apply #'concatenate 'string
         (loop for child in (get-children node)
            collect (cond ((stringp child) child)
                          (t (get-trimmed-text child))))))

(defun edit-distance (a b)
  (let ((cost (make-array (list (1+ (length a))
                                (1+ (length b)))
                          :initial-element 0)))
    (loop for i below (1+ (length a))
       do (loop for j below (1+ (length b))
             do (setf (aref cost i j)
                      (cond ((zerop i) j)
                            ((zerop j) i)
                            (t (min (1+ (aref cost (1- i) j))
                                    (1+ (aref cost i (1- j)))
                                    (+ (if (char= (aref a (1- i)) (aref b (1- j)))
                                           0 1)
                                       (aref cost (1- i) (1- j)))))))))
    (aref cost (length a) (length b))))

(defun closest (key-func target input-list)
  (loop with min-distance = (edit-distance target 
                                           (funcall key-func (car input-list)))
     and best-item = (car input-list)
     for item in (rest input-list)
     do (let ((distance (edit-distance target 
                                       (funcall key-func item))))
          (when (< distance min-distance)
            (setf best-item item)
            (setf min-distance distance)))
     finally (return (when (or (<= min-distance 1)
                               (<= min-distance (* 0.5 (length target))))
                       best-item))))

;; (def-struct-wrapper crawl-4u-armor-index
;;   ("td:1 b a" :link #'get-href))

;; (def-list-wrapper crawl-4u-armor-indices
;;   (#.(mkstr "body .WikiaSiteWrapper #WikiaPage .WikiaPageContentWrapper "
;;             "#WikiaMainContent #WikiaMainContentContainer #WikiaArticle "
;;             "#mw-content-text div div[title]:1 table tbody tr");; .tabbertab")
;;    :test-page
;;    (lambda (node) node)))

;;; ---------- Skill Crawler ----------

(def-struct-wrapper crawl-4u-skill-table-entry
  ("td:1 h3 span" :en-name #'get-trimmed-content)
  ;; If there is a link to the name, this entry will overwrite the
  ;; previous one.
  ("td[rowspan]:1 h3 span a span" :en-name #'get-trimmed-content)
  ("td[rowspan]:1" :jp-name #'get-trimmed-text)
  ("td[~rowspan]:1" :jp-skill-name #'get-trimmed-text)
  ("td[~rowspan]:1" :en-skill-name 
                    (lambda (node)
                      (string-trim-all (nth 2 (get-children node)))))
  ("td[~rowspan]:2" :points #'get-trimmed-content-int)
  ("td[~rowspan]:3" :en-description #'get-trimmed-content)
  ("td[~rowspan]:3 i" :en-description #'get-trimmed-paragraph))
                           


(def-list-wrapper crawl-4u-skill-table
    "tr:>1"
  #'crawl-4u-skill-table-entry)

(def-list-wrapper crawl-4u-skill-systems
    #.(mkstr "body .WikiaSiteWrapper #WikiaPage .WikiaPageContentWrapper "
             "#WikiaMainContent #WikiaMainContentContainer #WikiaArticle "
             "#mw-content-text table:>3 tbody")
    #'crawl-4u-skill-table)
      
  
(defun update-4u-skills ()
  (labels ((form-skill-obj (current)
             (list :obj t
                   :en-name (getf current :en-skill-name)
                   :jp-name (getf current :jp-skill-name)
                   :points  (if (string= (string-downcase (getf current :en-skill-name))
                                         "torso up")
                                0
                                (getf current :points))
                   :en-description (getf current :en-description))))
    (with-open-file (*standard-output* (get-dataset-file-name "skills-4u.lisp")
                                       :direction :output
                                       :if-exists :supersede)
      (write (reduce (lambda (current accu)
                       (if (getf current :en-name)
                           (cons (list :obj t
                                       :en-name (getf current :en-name)
                                       :jp-name (getf current :jp-name)
                                       :skills (list (form-skill-obj current)))
                                 accu)
                           (let ((skill-system (car accu)))
                             (push (form-skill-obj current)
                                   (getf skill-system :skills))
                             (cons skill-system (cdr accu)))))
                     (mapcan #'identity 
                             (crawl-4u-skill-systems (html-from-uri *skill-list-url*)))
                     :initial-value nil
                     :from-end t))))
  (format t "successfully updated skill-4u.lisp.~%"))

(defun update-skill-correspondence ()
  (labels ((read-skills-data ()
             (with-open-file (*standard-input* (get-dataset-file-name "skills-4g.lisp")
                                               :direction :input
                                               :if-does-not-exist :error)
               (read)))
           (read-4u-skills-data ()
             (with-open-file (*standard-input* (get-dataset-file-name "skills-4u.lisp")
                                               :direction :input
                                               :if-does-not-exist :error)
               (read)))
           (extend-skills (skills en-skills)
             (mapcar (lambda (skill)
                       (aif (find-if #`,(= (getf x1 :points)
                                           (getf skill :points))
                                     en-skills)
                            (append (list :en-name (getf it :en-name)
                                          :en-description (getf it :en-description))
                                    skill)
                            (format t "~a(~a) not matched.~%"
                                    (getf skill :name)
                                    (getf skill :points))))
                     skills))
           (extend-skill-system (system en-system)
             (setf (getf system :en-name)
                   (getf en-system :en-name))
             (setf (getf system :skills)
                   (extend-skills (getf system :skills)
                                  (getf en-system :skills)))
             system))
    (let ((jp-data (read-skills-data))
          (en-data (read-4u-skills-data))
          (not-found-count 0))
      (let ((new-data (mapcar (lambda (system)
                                (let* ((system-name (getf system :system-name))
                                       (matched (closest #`,(getf x1 :jp-name)
                                                         system-name
                                                         en-data)))
                                  (if matched
                                      (progn (format t "[ ok ] ~a: ~a~%" 
                                                     system-name
                                                     (getf matched :en-name))
                                             (extend-skill-system system matched))
                                      (progn (format t "[ xx ] ~a: not found.~%" system-name)
                                             (incf not-found-count)
                                             system))))
                              jp-data)))
        (format t "4G Skills: ~a~%" (length jp-data))
        (format t "4U Skills: ~a~%" (length en-data))
        (format t "Total not found: ~a~%" not-found-count)
        (with-open-file (*standard-output* (get-dataset-file-name "skills.lisp")
                                           :direction :output
                                           :if-exists :supersede)

          (write new-data)))))
  (format t "Skill correspondence updated.~%"))


              
                  
                             
