;;;; data-struct.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;;
;;;; Description: component of the package armor up, that defines the
;;;; data strcutres and utilities on of armors, skill systems and
;;;; jewels.

(in-package #:breakds.monster-avengers.armor-up)

;;; ---------- Structures ----------

(defstruct skill-system
  (id 0 :type (unsigned-byte 32))
  (skills nil))

(defstruct armor
  (id 0 :type (unsigned-byte 32))
  (part-id 0 :type (unsigned-byte 32))
  (effects nil)
  (holes 0 :type (unsigned-byte 32))
  (defense 0 :type (unsigned-byte 32))
  (type "melee" :type string))

(defstruct jewel
  (id 0 :type (unsigned-byte 32))
  (holes 0 :type (unsigned-byte 32))
  (effects nil))

;;; ---------- Utilities ----------


;; (declaim (inline points-of-skill))
(defun points-of-skill (armor-piece skill-id)
  (aif (assoc skill-id
	      (armor-effects armor-piece))
       (the fixnum (cadr it))
       (the fixnum 0)))



;;; ---------- Globals and Specials ----------

(defparameter *skill-systems* nil
  "Array that stores all the skill systems.")

(defparameter *skill-systems-indices* nil
  "Hash-table that maps skill name to its id.")

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

(defparameter *jewels* nil
  "Array that stores all the jewels.")

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



  
  
  


  
