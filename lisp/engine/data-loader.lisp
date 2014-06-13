;;;; data-loader.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;;
;;;; Description: component of the package armor up, that provides
;;;; routines to load data for a particular game (e.g. Monster Hunter
;;;; 4), including data for different languages.

(in-package #:breakds.monster-avengers.armor-up)

(defun translate-language-buffer (lang-buf)
  (setf *name-packages* nil)
  ;; get list of languages
  (loop for item in lang-buf
     do (when (not (assoc (second item) *name-packages* :test #'equal))
	  (push (list (second item) (make-name-pkg))
		*name-packages*)))
  (let ((dim-skill-systems 0)
	(dim-armors 0)
	(dim-parts 0)
	(dim-jewels 0))
    ;; get dimensions
    (loop for item in lang-buf
       do (case (first item)
	    (armor (progn (when (> (third item) dim-parts)
			    (setf dim-parts (third item)))
			  (when (> (fourth item) dim-armors)
			    (setf dim-armors (fourth item)))))
	    (jewel (when (> (third item) dim-jewels)
		     (setf dim-jewels (third item))))
	    (skill-system (when (> (third item) dim-skill-systems)
			    (setf dim-skill-systems (third item))))))
    ;; update dimensions
    (loop for (key val) in *name-packages*
       do (progn (setf (name-pkg-skill-system val)
		       (make-array (+ 50 dim-skill-systems)
				   :element-type 'string
				   :initial-element ""))
		 (setf (name-pkg-jewel val)
		       (make-array (+ 50 dim-jewels)
				   :element-type 'string
				   :initial-element ""))
		 (setf (name-pkg-armor val)
		       (make-array (list (+ 3 dim-parts)
					 (+ 50 dim-armors))
				   :element-type 'string
				   :initial-element ""))))
    ;; update packages
    (macrolet ((get-cell (language type &rest indices)
		 `(aref (,(symb 'name-pkg- type) 
			  (second (assoc ,language
					 *name-packages*
					 :test #'equal)))
			,@indices)))
      (loop for item in lang-buf
	 do (case (first item)
	      (armor (setf (get-cell (second item) armor 
				     (third item) (fourth item))
			   (fifth item)))
	      (jewel (setf (get-cell (second item) jewel (third item))
			   (fourth item)))
	      (skill-system (setf (get-cell (second item) skill-system 
					    (third item))
				  (fourth item)))))))
  (setf *name-package* (cadar *name-packages*)))

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
  (with-gensyms (file-name item)
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
	   (translate-language-buffer language-buffer)
	   (format t "[ok] Data set [~a] has been successfully loaded.~%" ,title)
	   (format t "Monster avengers, armor up!~%"))))))


;;; ---------- Monster Hunter 4 ----------

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
				 collect (list (gethash (getf pair :skill-name)
							jap-name-to-id)
					       (getf pair :skill-point)))))))
      (format t "[ok] Jewels loaded.~%"))))


(define-data-set-loader mhp3 ("Monster Hunter P3")
  (load-data-file *skill-systems* (row :from (access-file "skills.lisp"))
    (skill-system (:name-chinese <- (getf row :system-name))
		  (:skills <- (getf row :skills))))
  (format t "[ok] Skill system loaded.~%")
  ;; Create skill-system chinese-name to id map 
  (let ((chn-name-to-id (make-hash-table :test #'equal)))
    (loop for item in language-buffer
       when (string-equal (second item) "chinese")
       do (setf (gethash (fourth item) chn-name-to-id) (third item)))
    (macrolet 
	((load-armor (data-store file-name part-id)
	   `(progn
	      (load-data-file ,data-store (row :from (access-file ,file-name)
					       :when (getf row :name))
		(armor (:name-chinese <- (getf row :name))
		       (:part-id <- ,part-id)
		       (:holes <- (getf row :holes))
		       (:defense <- (getf row :defense))
		       (:type <- (getf row :type))
		       (:effects 
			<- (loop 
			      for points in (getf row :effective-points)
			      for skill-name in (getf row :effective-skills)
			      collect (list (gethash skill-name chn-name-to-id)
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
	(jewel (:name-chinese <- (getf row :name))
	       (:holes <- (getf row :holes))
	       (:effects <- (progn 
			      (loop for pair in (getf row :effects)
				 when (not (empty-struct-p pair))
				 collect (list (gethash (getf pair :skill-name)
							chn-name-to-id)
					       (getf pair :skill-point)))))))
      (format t "[ok] Jewels loaded.~%"))))








