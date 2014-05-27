;;;; mhp3-crawler.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;;
;;;; Description: Crawl http://mhp3wiki.duowan.com/ for armor data and 

(in-package #:breakds.monster-avengers.mhp3-crawler)

(defparameter *root-domain* "http://mhp3wiki.duowan.com")

(declaim (inline string-trim-all))
(defun string-trim-all (str)
  (string-trim '(#\space #\newline #\tab) str))

(declaim (inline get-trimmed-content))
(defun get-trimmed-content (node)
  (string-trim-all (get-content node)))

(declaim (inline get-trimmed-content-int))
(defun get-trimmed-content-int (node)
  (parse-integer (string-trim-all (get-content node))))

(defmacro mapcan-list-wrapper ((node &key (base "") (sub-crawler nil)) &body body)
  `(append ,@(mapcar #`(funcall (make-list-wrapper ,(mkstr base " " 
                                                           (car x1))
                                                   ,sub-crawler)
                                ,node)
                     body)))
  

;;; ---------- Skill System Wrapper ----------

(def-struct-wrapper crawl-skill-system
  ("body #globalWrapper #column-content #content #bodyContent table tbody tr:2 td p strong"
   :system-name (lambda (node) (string-trim-all (get-content node))))
  ("body #globalWrapper #column-content #content #bodyContent table tbody"
   :skills (make-list-wrapper "tr:>1" 
                              (lambda (node)
                                (let ((rev-tds (nreverse 
                                                (mapcar #'get-trimmed-content
                                                        (get-children node))))
                                      (result '(:obj t)))
                                  (setf (getf result :name) (nth 1 rev-tds))
                                  (setf (getf result :points) 
                                        (handler-case
                                            (parse-integer (nth 2 rev-tds))
                                          (t (e) (declare (ignore e)) 0)))
                                  (setf (getf result :description) (nth 0 rev-tds))
                                  result)))))
                                        

(defun crawl-skill-systems (node)
  (mapcan-list-wrapper (node :base "body #globalWrapper #column-content #content #bodyContent font"
                             :sub-crawler (lambda (x)
                                            (format t "Crawling ~a ...~%" (get-trimmed-content x))
                                            (crawl-skill-system 
                                             (html-from-uri 
                                              (format nil "~a~a"
                                                      *root-domain*
                                                      (second (assoc 
                                                               "href" 
                                                               (get-attributes x)
                                                               :test #'equal)))))))
    ("table tbody tr td p a")
    ("table tbody tr td a")
    ("font table tbody tr td p a")
    ("font font table tbody tr td p a")
    ("font font font table tbody tr td p a")
    ("font font font font table tbody tr td a")
    ("font font font font font table tbody tr td a")
    ("font font font font font font table tbody tr td a")
    ("font font font font font font font table tbody tr td a")
    ("font font font font font font font font table tbody tr td a")))

  
;;; ---------- Armor Wrapper ----------

(def-list-wrapper crawl-armors
    "body #globalWrapper #column-content #content #bodyContent table tbody tr:>2"
  (make-struct-wrapper
   ("td:1 a" :name #'get-trimmed-content)
   ("td:2" :rank #'get-trimmed-content-int)
   ("td:3" :type (lambda (x) (let ((text (get-trimmed-content x)))
                               (cond ((equal text "剑士") "melee")
                                     ((equal text "射手") "range")
                                     (t "both")))))
   ("td:4" :defense (lambda (x) (let ((text (get-trimmed-content x)))
                                  (parse-integer (subseq text 
                                                         (1+ (position #\/ text)))))))
   ("td:5" :holes (lambda (x) (count #\O (get-trimmed-content x))))
   ("td:7 ul" :effective-skills (make-list-wrapper "li a" #'get-trimmed-content))
   ("td:7 ul" :effective-points (make-list-wrapper "li font" #'get-trimmed-content-int))))

;;; ---------- Jewels Wrapper ----------

(def-list-wrapper crawl-jewels
    "body #globalWrapper #column-content #content #bodyContent table tbody tr[bgcolor]"
  (make-struct-wrapper
   ("td:1 a" :name #'get-trimmed-content)))

   

           

  


