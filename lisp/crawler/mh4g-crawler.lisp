;;;; mh4-crawler.lisp
;;;; author: BreakDS <breakds@gmail.com>

(in-package #:breakds.monster-avengers.mh4g-crawler)

(defparameter *root-domain* "http://wiki.mh4g.org/")

;;; ---------- Globals ----------

(defparameter *skills* nil)
(defparameter *helms* nil)
(defparameter *cuirasses* nil)
(defparameter *gloves* nil)
(defparameter *cuisses* nil)
(defparameter *sabatons* nil)
(defparameter *jewels* nil)

;;; ---------- Skill Crawler ----------
(declaim (inline string-trim-all))
(defun string-trim-all (str)
  (string-trim '(#\space #\newline #\tab) str))

(declaim (inline get-trimmed-content))
(defun get-trimmed-content (node)
  (string-trim-all (get-content node)))

(declaim (inline get-trimmed-content-int))
(defun get-trimmed-content-int (node)
  (parse-integer (string-trim-all (get-content node))))

(defun crawl-skill (node)
  (let ((count 0)
        (result '(:obj t)))
    (loop for child in (get-children node)
       when (and (string-equal (get-tag child) "td")
                 (null (assoc "rowspan" (get-attributes child)
                              :test #'equal)))
       do (incf count)
         (case count
           (1 (setf (getf result :name) (get-trimmed-content child)))
           (2 (setf (getf result :points) (get-trimmed-content-int child)))
           (3 (setf (getf result :description) (get-trimmed-content child)))))
    result))


(def-struct-wrapper crawl-skill-system
  ("body #wrapper #container #contents #data_container table:1 tbody tr td #data2 table:1 tbody tr:2 td:1"
   :system-name (lambda (node) 
                  (string-trim-all (get-content node))))
  ("body #wrapper #container #contents #data_container table:1 tbody tr td #data2 table:1 tbody"
   :skills (make-list-wrapper "tr:>1" #'crawl-skill)))

(def-list-wrapper crawl-skill-systems
    "body #wrapper #container #contents #data_container table:1 tbody tr td #data2 table tbody tr td a"
  (lambda (node)
    (format t "crawling ~a ... ~%" (get-trimmed-content node))
    (let ((href (cadr (assoc "href" (get-attributes node)
                             :test #'equal))))
      (crawl-skill-system (html-from-uri (format nil
                                                 "~a~a"
                                                 *root-domain*
                                                 href))))))

;;; ---------- Jewels Crawler ----------

(def-struct-wrapper crawl-jewel
  ("tr:1 td:1 a" :name (lambda (node)
                         (let ((name (get-trimmed-content node)))
                           (format t "crawling ~a ...~%" name)
                           name)))
  ("tr:1 td:1 span" :holes (lambda (node)
			     (- 3 (count #\-
					 (get-trimmed-content node)))))
  ("" :effects (make-list-wrapper
  		"tr td[~rowspan]"
  		(make-struct-wrapper
  		 ("a" :skill-name #'get-trimmed-content)
  		 ("span" :skill-point #'get-trimmed-content-int)))))

(def-list-wrapper crawl-jewels-list
    "body #wrapper #container #contents #data_container table:1 tbody tr td #data2 table tbody"
  #'crawl-jewel)


;;; ---------- Armor Crawler ----------

(defmacro def-armor-wrapper (part-name (type first-tr-id second-tr-id))
  `(progn
     (def-struct-wrapper ,(symb 'crawl- (string-upcase type) "-" part-name)
       ("" :type (lambda (node)
		   (declare (ignorable node))
		   ,type))
       (,(mkstr "body #wrapper #container #contents #data_container table:1 tbody " 
                "tr td #data2 table:1 tbody tr:" first-tr-id " td[~rowspan]:2 not(img):2")
	 :name #'string-trim-all)
       (,(mkstr "body #wrapper #container #contents #data_container table:1 tbody "
                "tr td #data2 table:1 tbody tr:" first-tr-id " td[~rowspan]:4")
         :rank #'get-trimmed-content-int)
       (,(mkstr "body #wrapper #container #contents #data_container table:1 tbody "
                "tr td #data2 table:1 tbody tr:" first-tr-id " td[~rowspan]:11")
         :defense (lambda (node) (handler-case (get-trimmed-content-int node)
                                   (t () 0))))
       (,(mkstr "body #wrapper #container #contents #data_container table:1 tbody "
                "tr td #data2 table:3 tbody tr:" second-tr-id " td[~rowspan]:2")
         :holes #`,(- 3 (count #\- (get-trimmed-content x1))))
       (,(mkstr "body #wrapper #container #contents #data_container table:1 tbody "
                "tr td #data2 table:3 tbody tr:" second-tr-id " td:3")
         :effective-skills (make-list-wrapper "a" #'get-trimmed-content))
       (,(mkstr "body #wrapper #container #contents #data_container table:1 tbody "
                "tr td #data2 table:3 tbody tr:" second-tr-id " td:3")
         :effective-points (make-list-wrapper "span" #'get-trimmed-content-int))
       (,(mkstr "body #wrapper #container #contents #data_container table:1 tbody "
                "tr td #data2 table:4 tbody tr:" 
                (if (string= type "melee")
                    second-tr-id
                    (1- second-tr-id))
                " td:2")
         :material (make-list-wrapper "a" #'get-trimmed-content)))
     (def-list-wrapper ,(symb 'crawl- (string-upcase type) "-" part-name '-list)
	 ,(mkstr "body #wrapper #container #contents #data_container table:1 tbody "
                 "tr td #data2 table tbody tr td a")
       (lambda (node)
	 (let ((href (cadr (assoc "href" (get-attributes node)
				  :test #'equal))))
           (format t "Crawling ~a ... ~a~a~%" 
                   (get-trimmed-content node)
                   *root-domain* href)
           (handler-case
               (,(symb 'crawl- (string-upcase type) "-" part-name)
                 (html-from-uri (format nil
                                        "~a~a"
                                        *root-domain*
                                        href)))
             (t () '(:obj t))))))))

	 

(def-armor-wrapper helm ("melee" 2 2))
(def-armor-wrapper cuirass ("melee" 3 3))
(def-armor-wrapper glove ("melee" 4 4))
(def-armor-wrapper cuisses ("melee" 5 5))
(def-armor-wrapper sabaton ("melee" 6 6))

(def-armor-wrapper helm ("range" 8 8))
(def-armor-wrapper cuirass ("range" 9 9))
(def-armor-wrapper glove ("range" 10 10))
(def-armor-wrapper cuisses ("range" 11 11))
(def-armor-wrapper sabaton ("range" 12 12))
   

;;; ---------- Exported Utilities ----------

(defun get-jap-file-name (file-name)
  (ensure-directories-exist
   (merge-pathnames (format nil "dataset/MH4G/~a" file-name)
                    (asdf:system-source-directory 'monster-avengers))))

(defparameter *skills-file* 
  (get-jap-file-name "skills.lisp"))

(defparameter *jewels-file* 
  (get-jap-file-name "jewels.lisp"))

(defparameter *helms-file* 
  (get-jap-file-name "helms.lisp"))

(defparameter *cuirasses-file* 
  (get-jap-file-name "cuirasses.lisp"))

(defparameter *gloves-file* 
  (get-jap-file-name "gloves.lisp"))

(defparameter *cuisses-file* 
  (get-jap-file-name "cuisses.lisp"))

(defparameter *sabatons-file* 
  (get-jap-file-name "sabatons.lisp"))

(defun update-jap-dataset (&key (force-refresh nil))
  (labels ((load-or-crawl (crawler file-name uri)
             (if (or force-refresh
                     (not (probe-file file-name)))
                 (let ((result (funcall crawler uri)))
                   (with-open-file (out file-name
                                        :direction :output
                                        :if-exists :supersede
                                        :if-does-not-exist :create)
                     (print result out))
                   result)
                 (with-open-file (in file-name
                                     :direction :input)
                   (read in)))))
    (setf *skills* 
          (load-or-crawl (lambda (uri)
                           (crawl-skill-systems
                            (html-from-uri uri)))
                         *skills-file*
                         "http://wiki.mh4g.org/data/1446.html"))
    (setf *jewels* 
          (load-or-crawl (lambda (uris)
                           (mapcan (lambda (uri)
                                     (crawl-jewels-list (html-from-uri uri)))
                                   uris))
                         *jewels-file*
                         '("http://wiki.mh4g.org/data/1477.html"
                           "http://wiki.mh4g.org/data/1478.html"
                           "http://wiki.mh4g.org/data/1479.html")))
    (setf *helms* 
          (load-or-crawl (lambda (uri)
                           (append (crawl-melee-helm-list (html-from-uri uri))
                                   (crawl-range-helm-list (html-from-uri uri))))
                         *helms-file*
                         "http://wiki.mh4g.org/data/1445.html"))
    (setf *cuirasses* 
          (load-or-crawl (lambda (uri)
                           (append (crawl-melee-cuirass-list (html-from-uri uri))
                                   (crawl-range-cuirass-list (html-from-uri uri))))
                         *cuirasses-file*
                         "http://wiki.mh4g.org/data/1445.html"))
    (setf *gloves* 
          (load-or-crawl (lambda (uri)
                           (append (crawl-melee-glove-list (html-from-uri uri))
                                   (crawl-range-glove-list (html-from-uri uri))))
                         *gloves-file*
                         "http://wiki.mh4g.org/data/1445.html"))
    (setf *cuisses* 
          (load-or-crawl (lambda (uri)
                           (append (crawl-melee-cuisses-list (html-from-uri uri))
                                   (crawl-range-cuisses-list (html-from-uri uri))))
                         *cuisses-file*
                         "http://wiki.mh4g.org/data/1445.html"))
    (setf *sabatons* 
          (load-or-crawl (lambda (uri)
                           (append (crawl-melee-sabaton-list (html-from-uri uri))
                                   (crawl-range-sabaton-list (html-from-uri uri))))
                         *sabatons-file*
                         "http://wiki.mh4g.org/data/1445.html"))))


  
  



      





    
