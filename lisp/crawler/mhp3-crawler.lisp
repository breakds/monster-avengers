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
  (handler-case 
      (parse-integer (string-trim-all (get-content node)))
    (t (e) (if (equal (get-trimmed-content node) "E")
               0
               (progn
                 (format t "warning: bad numebr while handling ~a~%" node)
                 nil)))))


       


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

(defun crawl-skill-systems-from (uri)
  (let ((skill-systems (crawl-skill-systems (html-from-uri uri))))
    (with-open-file (out (merge-pathnames "dataset/MHP3/skills.lisp"
                                          (asdf:system-source-directory 'monster-avengers))
                         :direction :output
                         :if-exists :supersede
                         :if-does-not-exist :create)
      (print skill-systems out)))
  (format t "[ok] Skill systems crawled.~%"))

  
;;; ---------- Armor Wrapper ----------

(def-list-wrapper crawl-armors
    "body #globalWrapper #column-content #content #bodyContent table tbody tr:>2"
  (make-struct-wrapper
   ("td:1 a" :name (lambda (x) 
                     (let ((name (get-trimmed-content x)))
                     (format t "Crawling armor ~a ...~%" name)
                     name)))
   ("td:2" :rank #'get-trimmed-content-int)
   ("td:3" :type (lambda (x) (let ((text (get-trimmed-content x)))
                               (cond ((equal text "剑士") "melee")
                                     ((equal text "射手") "range")
                                     (t "both")))))
   ("td:4" :defense (lambda (x) (let ((text (get-trimmed-content x)))
                                  (aif (position #\/ text)
                                       (parse-integer (subseq text (1+ it)))
                                       (parse-integer text)))))
   ("td:5" :holes (lambda (x) (count #\O (get-trimmed-content x))))
   ("" :effective-skills (lambda (node)
                           (aif (funcall (make-list-wrapper
                                          "td:7 ul li a"
                                          #'get-trimmed-content)
                                         node)
                                it
                                (funcall (make-list-wrapper
                                          "td:6 ul li a"
                                          #'get-trimmed-content)
                                         node))))
   ("" :effective-points (lambda (node)
                           (aif (remove-if #'null 
                                           (funcall (make-list-wrapper 
                                                     "td:7 ul li font:1" 
                                                     #'get-trimmed-content-int)
                                                    node))
                                it
                                (aif (remove-if #'null 
                                                (funcall (make-list-wrapper 
                                                          "td:7 ul li span font:1" 
                                                          #'get-trimmed-content-int)
                                                         node))
                                     it
                                     (aif (remove-if #'null
                                                     (funcall (make-list-wrapper
                                                               "td:6 ul li font:1"
                                                               #'get-trimmed-content-int)
                                                              node))
                                          it
                                          (remove-if #'null (funcall 
                                                             (make-list-wrapper
                                                              "td:6 ul li span font:1"
                                                              #'get-trimmed-content-int)
                                                             node)))))))))
(defun crawl-armors-from (uri part)
  (let ((armors (crawl-armors (html-from-uri uri))))
    (with-open-file (out (merge-pathnames (format nil "dataset/MHP3/~a.lisp" part)
                                          (asdf:system-source-directory 'monster-avengers))
                         :direction :output
                         :if-exists :supersede
                         :if-does-not-exist :create)
      (print armors out)))
  (format t "[ok] Armor: ~a crawled.~%" part))
  

;;; ---------- Jewels Wrapper ----------

(def-list-wrapper crawl-jewels
    "body #globalWrapper #column-content #content #bodyContent table tbody tr[bgcolor]"
  (lambda (node)
    (when (> (length (get-children node)) 4)
      (funcall (make-struct-wrapper
                ("td:1 a" :name (lambda (x)
                                  (let ((name (get-trimmed-content x)))
                                    (format t "Crawling jewel ~a ... ~%" name)
                                    name)))
                ("td:2" :effects (make-list-wrapper 
                                  "font"
                                  (make-struct-wrapper
                                   ("a" :skill-name #'get-trimmed-content)
                                   ("b" :skill-point #'get-trimmed-content-int))))
                ("td:3" :holes (lambda (x) (count #\O (get-trimmed-content x)))))
               node))))

(defun crawl-jewels-from (uri)
  (let ((jewels (remove-if #'null (crawl-jewels (html-from-uri uri)))))
    (with-open-file (out (merge-pathnames "dataset/MHP3/jewels.lisp"
                                          (asdf:system-source-directory 'monster-avengers))
                         :direction :output
                         :if-exists :supersede
                         :if-does-not-exist :create)
      (print jewels out)))
  (format t "[ok] Jewels crawled.~%"))


;;; ---------- Crawler ----------

(defun refresh-mhp3-dataset ()
  ;; (crawl-skill-systems-from "http://mhp3wiki.duowan.com/%E6%8A%80%E8%83%BD?variant=zh-hans")
  (crawl-armors-from "http://mhp3wiki.duowan.com/%E5%A4%B4%E9%98%B2%E5%85%B7?variant=zh-hans"
                     "helms")
  (crawl-armors-from "http://mhp3wiki.duowan.com/%E8%83%B4%E9%98%B2%E5%85%B7?variant=zh-hans"
                     "cuirasses")
  (crawl-armors-from "http://mhp3wiki.duowan.com/%E8%85%95%E9%98%B2%E5%85%B7?variant=zh-hans"
                     "gloves")
  (crawl-armors-from "http://mhp3wiki.duowan.com/%E8%85%B0%E9%98%B2%E5%85%B7?variant=zh-hans"
                     "cuisses")
  (crawl-armors-from "http://mhp3wiki.duowan.com/%E8%84%9A%E9%98%B2%E5%85%B7?variant=zh-hans"
                     "sabatons")
  (crawl-jewels-from "http://mhp3wiki.duowan.com/%E8%A3%85%E9%A5%B0%E7%8F%A0?variant=zh-hans")
  (format t "[ok] Dataset Monster Hunter P3 crawled successfully.~%"))


   

           

  


