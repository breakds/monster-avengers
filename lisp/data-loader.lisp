;;;; data-loader.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;;
;;;; Description: component of the package armor up, that provides
;;;; routines to load data for a particular game (e.g. Monster Hunter
;;;; 4), including data for different languages.

(in-package #:breakds.monster-avengers.armor-up)

(defmacro load-data-file (data-store 
			 (row &key (from nil) (sorting nil) (when nil))
			 &body body)
  "Use this macro inside define-data-set-loader only."
  (when (null from)
    (error "Please provide the input file for the dataset."))
  (when (not (member (caar body) '(armor jewel skill-system)))
    (error "Data type not within (armor jewl skill-system)"))
  (with-gensyms (input elements id constructed)
    (labels ((gen-make-struct (body id-symb)
	       (append (list (symb 'make- (caar body))
			     :id `(incf ,id-symb))
		       (remove-if 
			#`,(eq x1 '<-)
			(mapcan #`(,(first x1) ,(third x1))
				(remove-if #`,(string-equal 
					       (subseq (mkstr (car x1)) 0 4)
					       "name")
					   (cdar body))))))
	     (get-language (name-symb)
	       (let ((str (mkstr name-symb)))
		 (when (and (>= (length str) 4)
			    (string-equal (subseq str 0 4) "name"))
		   (subseq str (1+ (position #\- str))))))

	     (gen-language-pusher (body)
	       (loop 
		  for item in (cdar body)
		  for language = (get-language (car item))
		  when language
		  collect (case (caar body)
			    (armor `(push (list 'armor 
						,language
						(armor-part-id ,constructed)
						(armor-id ,constructed)
						,(third item))
					  language-buffer))
			    (jewel `(push (list 'jewel
						,language
						(jewel-id ,constructed)
						,(third item))
					  language-buffer))
			    (skill-system 
			     `(push (list 'skill-system
					  ,language
					  (skill-system-id ,constructed)
					  ,(third item))
				    language-buffer)))))
	     (gen-body (body id-symb)
	       `(let ((,constructed ,(gen-make-struct body id-symb)))
		  ,@(gen-language-pusher body)
		  ,constructed))
	     (get-element-type (body)
	       (caar body)))
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

(defmacro define-data-set-loader (name (title) &body body)
  (with-gensyms (file-name)
    `(defun ,(symb 'load- name '-data-set) ()
       ;; Anarchy local variables LANGUAGE-BUFFER 
       (let ((language-buffer nil))
	 ;; Anarchy local function ACCESS-FILE
	 (labels ((access-file (,file-name)
		    (merge-pathnames (format nil "dataset/~a/~a" 
					     ,(mkstr name) ,file-name)
				     (asdf:system-source-directory 
				      'monster-avengers))))
	   ,@body
	   (format t "[ok] Data set [~a] has been successfully loaded.~%" ,title)
	   (format t "Monster avengers, armor up!~%"))))))

(define-data-set-loader mh4 ("Monster Hunter 4")
  (load-data-file *skill-systems* (row :from (access-file "skills.lisp"))
    (skill-system (:name-japanese <- (getf row :system-name))
		  (:skills <- (getf row :skills))))
  (format t "[ok] Skill system loaded.~%")
  ;; Create skill-system japanese-name to id map 
  (let ((jap-name-to-id (make-hash-table :test #'equal)))
    (loop for item in language-buffer
       when (string-equal (second item) "japanese")
       do (setf (gethash (fourth item) jap-name-to-id) (third item)))
    (macrolet 
	((load-armor (data-store file-name part-id)
	   `(progn
	      (load-data-file ,data-store (row :from (access-file ,file-name)
					       :when (getf row :name))
		(armor (:name-japanese <- (getf row :name))
		       (:part-id <- ,part-id)
		       (:holes <- (getf row :holes))
		       (:defense <- (getf row :defense))
		       (:type <- (getf row :type))
		       (:effects 
			<- (loop 
			      for points in (getf row :effective-points)
			      for skill-name in (getf row :effective-skills)
			      collect (list (gethash skill-name jap-name-to-id)
					    points)))))
	      (format t "[ok] ~a loaded.~%" ,(mkstr data-store)))))
      (load-armor *helms* "helms.lisp" 0)
      (load-armor *cuirasses* "cuirasses.lisp" 1)
      (load-armor *gloves* "gloves.lisp" 2)
      (load-armor *cuisses* "cuisses.lisp" 3)
      (load-armor *sabatons* "sabatons.lisp" 4)
      (loop for item across *helms* do (setf (armor-type item) "both"))
      (load-data-file *jewels* (row :from (access-file "jewels.lisp")
				    :sorting (lambda (x y)
					       (< (getf x :holes)
						  (getf y :holes))))
	(jewel (:name-japanese <- (getf row :name))
	       (:holes <- (getf row :holes))
	       (:effects <- (progn 
			      (loop for pair in (getf row :effects)
				 when (not (empty-struct-p pair))
				 do (print (list (getf row :name)
						 (gethash (getf pair :skill-name)
							  jap-name-to-id)
						 (getf pair :skill-name)))
				 collect (list (gethash (getf pair :skill-name)
							jap-name-to-id)
					       (getf pair :skill-point)))))))
      (format t "[ok] Jewels loaded.~%"))))
						    


      
		     

  
  

  
	 
	 

       
      
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







