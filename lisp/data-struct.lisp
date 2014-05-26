;;;; data-struct.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;;
;;;; Description: component of the package armor up, that defines the
;;;; data strcutres and utilities on of armors, skill systems and
;;;; jewels.

(in-package #:breakds.monster-avengers.armor-up)

;;; ---------- Utilities ----------

(defun get-file-name (lang file-name)
  (merge-pathnames (format nil "dataset/~a/~a" lang file-name)
                   (asdf:system-source-directory 'monster-avengers)))

(defstruct skill-system
  (id 0 :type (unsigned-byte 32))
  (name "" :type string)
  (skills nil))

(defparameter *skill-systems* nil
  "Array that stores all the skill systems.")

(defparameter *skill-systems-indices* nil
  "Hash-table that maps skill name to its id.")

(defun load-skill-systems (&key (lang "jap"))
  (with-open-file (in (get-file-name lang "skills/skills.lisp")
                      :direction :input
                      :if-does-not-exist :error)
    (let ((elements 
           (loop 
              for entry in (read in)
              for id from 0
              collect (make-skill-system :id id
                                         :name (getf entry :system-name)
                                         :skills (getf entry :skills)))))
      (setf *skill-systems*
            (make-array (length elements)
                        :element-type 'skill-system
                        :initial-contents elements))
      (setf *skill-systems-indices*
            (make-hash-table :test #'equal))
      (loop for element in elements
         do (setf (gethash (skill-system-name element) 
                           *skill-systems-indices*)
                  (skill-system-id element))))
    (format t "[ok] Skill system loaded.~%")
    t))

(defun skill-system-id-from-name (name)
  (gethash name *skill-systems-indices*))


;;; ---------- Armors ----------

(defstruct armor
  (id 0 :type (unsigned-byte 32))
  (part-id 0 :type (unsigned-byte 32))
  (name "" :type string)
  (effects nil)
  (holes 0 :type (unsigned-byte 32))
  (defense 0 :type (unsigned-byte 32))
  (type "melee" :type string))

(defparameter *helms* nil
  "Array that stores all the helms.")

(defparameter *cuirasses* nil
  "Array that stores all the cuirasses.")

(defparameter *gloves* nil
  "Array that stores all the gloves.")

(defparameter *cuisses* nil
  "Array that stores all the cuisses.")

(defparameter *sabatons* nil
  "Array that stores all the sabatons.")

(defun load-armor-list (file-name part-id &key (lang "jap"))
  (with-open-file (in (get-file-name lang file-name)
                      :direction :input
                      :if-does-not-exist :error)
    (let* ((id -1)
           (elements
            (loop for entry in (read in)
               when (getf entry :name)
               collect 
                 (progn
                   (incf id)
                   (make-armor :id id
                               :name (getf entry :name)
                               :part-id part-id
                               :holes (getf entry :holes)
                               :defense (getf entry :defense)
                               :type (getf entry :type)
                               :effects (loop 
                                           for points in 
                                             (getf entry :effective-points)
                                           for skill-name in 
                                             (getf entry :effective-skills)
                                           collect 
                                             (list (skill-system-id-from-name 
                                                    skill-name)
                                                   points)))))))
      (make-array (length elements)
                  :element-type 'armor
                  :initial-contents elements))))
(defun load-helms (&key (lang "jap"))
  (setf *helms* (load-armor-list "armors/helms.lisp"
                                 0
                                 :lang lang))
  ;; hard code fix helms so that their type are both
  (loop for item across *helms*
     do (setf (armor-type item) "both"))
  (format t "[ok] Helms loaded.~%"))

(defun load-cuirasses (&key (lang "jap"))
  (setf *cuirasses* (load-armor-list "armors/cuirasses.lisp"
                                     1
                                     :lang lang))
  (format t "[ok] Cuirasses loaded.~%"))

(defun load-gloves (&key (lang "jap"))
  (setf *gloves* (load-armor-list "armors/gloves.lisp"
                                  2
                                  :lang lang))
  (format t "[ok] Gloves loaded.~%"))

(defun load-cuisses (&key (lang "jap"))
  (setf *cuisses* (load-armor-list "armors/cuisses.lisp"
                                   3
                                   :lang lang))
  (format t "[ok] Cuisses loaded.~%"))

(defun load-sabatons (&key (lang "jap"))
  (setf *sabatons* (load-armor-list "armors/sabatons.lisp"
                                    4
                                    :lang lang))
  (format t "[ok] Sabatons loaded.~%"))

;; (declaim (inline points-of-skill))
(defun points-of-skill (armor-piece skill-id)
  (aif (assoc skill-id
	      (armor-effects armor-piece))
       (the fixnum (cadr it))
       (the fixnum 0)))

;;; ---------- Jewels ----------

(defstruct jewel
  (id 0 :type (unsigned-byte 32))
  (name "" :type string)
  (holes 0 :type (unsigned-byte 32))
  (effects nil))

(defvar *jewels* nil "array of all the jewels")

(defun load-jewels (&key (lang "jap"))
  (with-open-file (in (get-file-name lang "jewels/jewels.lisp")
                      :direction :input
                      :if-does-not-exist :error)
    (let ((elements 
           (loop 
              for entry in (read in)
              for id from 0
              collect 
                (make-jewel 
		 :id id
		 :name (getf entry :name)
		 :holes (getf entry :holes)
		 :effects (loop for pair in 
			       (getf entry :effects)
			     when (not (empty-struct-p pair))
			     collect (list (skill-system-id-from-name
					    (getf pair :skill-name))
					   (getf pair :skill-point)))))))
      (setf *jewels*
            (sort (make-array (length elements)
			      :element-type 'jewel
			      :initial-contents elements)
		  (lambda (x y)
		    (< (jewel-holes x) (jewel-holes y)))))
      (loop 
	 for item across *jewels* 
	 for i from 0
	 do (setf (jewel-id item) i))
      (format t "[ok] Jewels loaded.~%")
      nil)))

;;; ---------- Initialization ----------

(defun init (&optional (lang "jap"))
  (load-skill-systems :lang lang)
  (load-helms :lang lang)
  (load-cuirasses :lang lang)
  (load-gloves :lang lang)
  (load-cuisses :lang lang)
  (load-sabatons :lang lang)
  (load-jewels :lang lang)
  (format t "[ok] *** Monster Avengers, Armor Up! ***~%")
  nil)

;;; ---------- Localization ----------

(defstruct name-pkg
  (skill-system (make-array '(0) :element-type 'string :initial-element "")
		:type (simple-array string (*)))
  (armor (make-array '(0 0) :element-type 'string :initial-element "")
	  :type (simple-array string (* *)))
  (jewel (make-array '(0) :element-type 'string :initial-element "")
	  :type (simple-array string (*))))

(defparameter *name-packages* nil)
(defvar *name-package* nil)

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

(defmacro with-name-package ((language) &body body)
  `(let ((*name-package* (second (assoc ,language *name-packages*
					:test #'string-equal))))
     ,@body))

(defmethod get-name ((item armor))
  (aref (name-pkg-armor *name-package*) 
	(armor-part-id item)
	(armor-id item)))

(defmethod get-name ((item jewel))
  (aref (name-pkg-jewel *name-package*)
	(jewel-id item)))

(defmethod get-name ((item skill-system))
  (aref (name-pkg-skill-system *name-package*)
	(skill-system-id item)))



  
  
  


  
