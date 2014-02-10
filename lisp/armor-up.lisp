;;;; armor-up.lisp
;;;; Author: BreakDS <breakds@gmail.com>

(in-package #:breakds.monster-avengers.armor-up)

(defun get-file-name (lang file-name)
  (merge-pathnames (format nil "dataset/~a/~a" lang file-name)
                   (asdf:system-source-directory 'monster-avengers)))


;;; ---------- Skills ----------

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


;;; ---------- Jewels ----------

(defstruct jewel
  (id 0 :type (unsigned-byte 32))
  (name "" :type string)
  (holes 0 :type (unsigned-byte 32))
  (effects nil))

(defparameter *jewels* nil
  "array of all the jewels")

(defun load-jewels (&key (lang "jap"))
  (with-open-file (in (get-file-name lang "jewels/jewels.lisp")
                      :direction :input
                      :if-does-not-exist :error)
    (let ((elements 
           (loop 
              for entry in (read in)
              for id from 0
              collect 
                (make-jewel :id id
                            :name (getf entry :name)
                            :holes (getf entry :holes)
                            :effects (loop for pair in 
                                          (getf entry :effects)
                                        collect (list (skill-system-id-from-name
                                                       (getf pair :skill-name))
                                                      (getf pair :skill-point)))))))
      (setf *jewels*
            (make-array (length elements)
                        :element-type 'jewel
                        :initial-contents elements))
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


;;; ---------- State Encoding/Decoding ----------
(declaim (inline encode-hole-sig))
(defun encode-hole-sig (hole-sig)
  (the (unsigned-byte 64)
       (logior (ldb (byte 4 0) 
                    (the (signed-byte 8) (nth 0 hole-sig)))
               (ash (ldb (byte 4 0) 
                         (the (signed-byte 8) (nth 1 hole-sig))) 4)
               (ash (ldb (byte 4 0) 
                         (the (signed-byte 8) (nth 2 hole-sig))) 8))))

(declaim (inline decode-hole-sig))
(defun decode-hole-sig (key)
  (declare (type (unsigned-byte 64) key))
  (list (logand key #b1111)
        (logand (ash key -4) #b1111)
        (logand (ash key -8) #b1111)))

(declaim (inline encode-skill-sig))
(defun encode-skill-sig (skill-sig)
  (let ((result (the (unsigned-byte 64) 0)))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for points in skill-sig
       for offset from 12 by 6
       do (setf result 
                (logior result
                        (ash (ldb (byte 6 0) points) offset))))
    result))

(declaim (inline decode-skill-sig))
(defun decode-skill-sig (key n)
  (declare (type (unsigned-byte 64) key))
  (loop
     for i below n
     for offset from 12 by 6
     collect (logand (ash key (- offset)) #b111111)))

(declaim (inline decode-skill-sig-full))
(defun decode-skill-sig-full (key n)
  (declare (type (unsigned-byte 64) key))
  (loop
     for i below n
     for offset from 12 by 6
     collect (let ((x (logand (ash key (- offset)) #b111111)))
               (if (logbitp 5 x)
                   (dpb x (byte 6 0) -1)
                   x))))

(declaim (inline encode-sig))
(defun encode-sig (hole-sig skill-sig)
  (the (unsigned-byte 64)
       (logior (encode-hole-sig hole-sig)
               (encode-skill-sig skill-sig))))

(declaim (inline decode-sig))
(defun decode-sig (key n)
  (declare (type (unsigned-byte 64) key))
  (values (decode-hole-sig key)
          (decode-skill-sig key n)))

(declaim (inline decode-sig-full))
(defun decode-sig-full (key n)
  (declare (type (unsigned-byte 64) key))
  (values (decode-hole-sig key)
          (decode-skill-sig-full key n)))


;;; ---------- Search ----------

(defstruct armor-tree
  (left nil)
  (right nil))

(declaim (inline make-map))
(defun make-map ()
  "Create a hash table with unsigned 64 keys."
  (make-hash-table :test #'eq))


;; (defun rqeuired-skill-filter (required-skills armor-list)
;;   (let ((result-map (make-map)))
;;     (loop for item in armor-list
;;        do (let (
  


;; (defun search-core (required-effects)
;;   (let ((
  
  







  

  
                                                  
                                
                   
  
  
  
    


