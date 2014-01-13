;;;; crawler.lisp
;;;; author: BreakDS <breakds@gmail.com>

(in-package #:breakds.monster-avengers.crawler)

(defparameter *root-domain* "http://wiki.mh4g.org/")


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
  ("body #wrapper #container #contents #data_container #data2 table:1 tbody tr:2 td:1" 
   :system-name (lambda (node) 
                  (string-trim-all (get-content node))))
  ("body #wrapper #container #contents #data_container #data2 table:1 tbody"
   :skills (make-list-wrapper "tr:>1" #'crawl-skill)))

(def-list-wrapper crawl-skill-systems
    "body #wrapper #container #contents #data_container #data2 table tbody tr td a"
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
  ("tr:1 td:1 a" :name #'get-trimmed-content)
  ("tr:1 td:1 span" :holes (lambda (node)
			     (- 3 (count #\-
					 (get-trimmed-content node)))))
  ("" :effects (make-list-wrapper
  		"tr td[~rowspan]"
  		(make-struct-wrapper
  		 ("a" :skill-name #'get-trimmed-content)
  		 ("span" :skill-point #'get-trimmed-content-int)))))

(def-list-wrapper crawl-jewels-list
    "body #wrapper #container #contents #data_container #data2 table tbody"
  #'crawl-jewel)


;;; ---------- Armor Crawler ----------

(defmacro def-armor-wrapper (part-name (type first-tr-id second-tr-id))
  `(progn
     (def-struct-wrapper ,(symb 'crawl- (string-upcase type) "-" part-name)
       ("" :type (lambda (node)
		   (declare (ignorable node))
		   ,type))
       (,(mkstr "body #wrapper #container #contents #data_container #data2 table:1 tbody tr:" first-tr-id " td[~rowspan]:2 not(img):2")
	 :name #'string-trim-all)
       (,(mkstr "body #wrapper #container #contents #data_container #data2 table:1 tbody tr:" first-tr-id " td[~rowspan]:4")
	 :rank #'get-trimmed-content-int)
       (,(mkstr "body #wrapper #container #contents #data_container #data2 table:1 tbody tr:" first-tr-id " td[~rowspan]:10")
	 :defense #'get-trimmed-content-int)
       (,(mkstr "body #wrapper #container #contents #data_container #data2 table:2 tbody tr:" second-tr-id " td[~rowspan]:2")
	 :holes #`,(- 3 (count #\- (get-trimmed-content x1))))
       (,(mkstr "body #wrapper #container #contents #data_container #data2 table:2 tbody tr:" second-tr-id " td:3")
	 :effective-skills (make-list-wrapper "a" #'get-trimmed-content))
       (,(mkstr "body #wrapper #container #contents #data_container #data2 table:2 tbody tr:" second-tr-id " td:3")
	 :effective-points (make-list-wrapper "span" #'get-trimmed-content-int)))
     (def-list-wrapper ,(symb 'crawl- (string-upcase type) "-" part-name '-list)
	 "body #wrapper #container #contents #data_container #data2 table tbody tr td a"
       (lambda (node)
	 (let ((href (cadr (assoc "href" (get-attributes node)
				  :test #'equal))))
	   (,(symb 'crawl- (string-upcase type) "-" part-name)
	     (html-from-uri (format nil
				    "~a~a"
				    *root-domain*
				    href))))))))

	 
(def-armor-wrapper helm ("melee" 2 2))
(def-armor-wrapper cuirass ("melee" 3 3))
(def-armor-wrapper glove ("melee" 4 4))
(def-armor-wrapper cuisse ("melee" 5 5))
(def-armor-wrapper boot ("melee" 6 6))

(def-armor-wrapper helm ("range" 7 8))
(def-armor-wrapper cuirass ("range" 8 9))
(def-armor-wrapper glove ("range" 9 10))
(def-armor-wrapper cuisse ("range" 10 11))
(def-armor-wrapper boot ("range" 11 12))
   
    
