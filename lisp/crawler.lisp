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

