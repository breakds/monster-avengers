;;;; data-loader.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;;
;;;; Description: component of the package armor up, that provides
;;;; routines to load data for a particular game (e.g. Monster Hunter
;;;; 4), including data for different languages.

(in-package #:breakds.monster-avengers.armor-up)

(defmacro load-data-set (data-store 
			 (row &key (from nil) (sorting nil) (when nil))
			 &body body)
  (when (null from)
    (error "Please provide the input file for the dataset."))
  (labels ((gen-body (body id-symb)
	     `(progn (incf ,id-symb)
		     ,(append (list (symb 'make- (caar body))
				    :id id-symb)
			      (cdar body))))
	   (get-element-type (body)
	     (caar body)))
    (with-gensyms (input elements id)
      `(with-open-file (,input ,from
			       :direction :input
			       :if-does-not-exist :error)
	 (let* ((,id -1)
		(,elements (loop 
			      for ,row in ,(if sorting
					       `(sort (read ,input)
						      ,sorting)
					       `(read ,input))
			      when ,(aif when it t)
			      collect ,(gen-body body id))))
	   (setf ,data-store (make-array (length ,elements)
					 :element-type ',(get-element-type body)
					 :initial-contents ,elements)))))))

(defun test-load ()
  (load-data-set *skill-systems* (row :from "/home/breakds/pf/projects/monster-avengers/dataset/MH4/skills.lisp")
    (skill-system :name (getf row :system-name)
		  :skills (getf row :skills))))
	 
	 

       
      
;; (defparameter *helms* nil
;;   "Array that stores all the helms.")

;; (defparameter *cuirasses* nil
;;   "Array that stores all the cuirasses.")

;; (defparameter *gloves* nil
;;   "Array that stores all the gloves.")

;; (defparameter *cuisses* nil
;;   "Array that stores all the cuisses.")

;; (defparameter *sabatons* nil
;;   "Array that stores all the sabatons.")

;; (defparameter *skill-systems* nil
;;   "Array that stores all the skill systems.")

;; (defparameter *skill-systems-indices* nil
;;   "Hash-table that maps skill name to its id.")







