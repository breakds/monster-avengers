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
                                        when (not (empty-struct-p pair))
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

(declaim (inline hole-part))
(defun hole-part (key)
  (declare (type (unsigned-byte 64) key))
  (the (unsigned-byte 64) (ldb (byte 12 0) key)))

(declaim (inline encode-armor))
(defun encode-armor (armor-piece required-effects)
  (let ((hole-sig (make-list 3 :initial-element 0))
        (skill-sig (loop for (id points) in required-effects
                      collect (aif (assoc id (armor-effects armor-piece))
                                   (cadr it)
                                   0))))
    (when (> (armor-holes armor-piece) 0)
      (incf (nth (1- (armor-holes armor-piece)) hole-sig)))
    (encode-sig hole-sig skill-sig)))

;; (declaim (inline jewel-skill-sig))
(defun jewel-skill-sig (jewel-piece required-skill-ids)
  (mapcar #`,(aif (cadr (assoc x1 (jewel-effects jewel-piece)))
                  it
                  0)
          required-skill-ids))

(declaim (inline encode-jewel-if-satisfy))
(defun encode-jewel-if-satisfy (jewel-piece required-skill-ids)
  "Return the encoded key of a jewel if it has a positive points for
  at least one of the required skills. Return nill otherwise."
  (let ((skill-sig (jewel-skill-sig jewel-piece required-skill-ids)))
    (when (> (count-if #`,(> x1 0) skill-sig) 0)
      (encode-sig (case (jewel-holes jewel-piece)
                    (1 '(1 0 0))
                    (2 '(0 1 0))
                    (3 '(0 0 1)))
                  skill-sig))))

(declaim (inline encoded-+))
(defun encoded-+ (key-a key-b)
  (declare (type (unsigned-byte 64) key-a))
  (declare (type (unsigned-byte 64) key-b))
  (let ((result (the (unsigned-byte 64)
                     (+ (ldb (byte 12 0) key-a)
                        (ldb (byte 12 0) key-b)))))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for offset from 12 to 57 by 6
       do (setf (ldb (byte 6 offset) result)
                (+ (ldb (byte 6 offset) key-a)
                   (ldb (byte 6 offset) key-b))))
    result))

(declaim (inline encoded-skill-+))
(defun encoded-skill-+ (key-a key-b)
  (declare (type (unsigned-byte 64) key-a))
  (declare (type (unsigned-byte 64) key-b))
  (let ((result (the (unsigned-byte 64) 0)))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for offset from 12 to 57 by 6
       do (setf (ldb (byte 6 offset) result)
                (+ (ldb (byte 6 offset) key-a)
                   (ldb (byte 6 offset) key-b))))
    result))

;;; ---------- Utilities ----------

(declaim (inline make-map))
(defun make-map ()
  "Create a hash table with unsigned 64 keys."
  (make-hash-table :test #'eq))


;;; ---------- Jewel Combos ----------

(defstruct jewel-combo
  (key 0 :type (unsigned-byte 64))
  (jewels nil))

(declaim (inline merge-jewel-combo))
(defun merge-jewel-combo (combo-a combo-b)
  (make-jewel-combo :key (encoded-+ (jewel-combo-key combo-a)
                                    (jewel-combo-key combo-b))
                    :jewels (append (jewel-combo-jewels combo-a)
                                    (jewel-combo-jewels combo-b))))

(defun create-jewel-combos (required-effects)
  (let ((candidates (mapcar #`,(make-jewel-combo :key (encode-hole-sig x1)
                                                 :jewels nil)
                            '((0 0 1) (0 1 0) (1 0 0))))
        (required-skill-ids (mapcar #`,(car x1)
                                    required-effects)))
    (loop for piece across *jewels*
       do (awhen (encode-jewel-if-satisfy piece required-skill-ids)
            (push (make-jewel-combo :key (the (unsigned-byte 64) it)
                                    :jewels (list (jewel-id piece)))
                  candidates)))
    (let ((full-map (make-map))
          (hole-map (make-map)))
      ;; full-map is a mapping from encoded key to list of combos
      ;; hole-map is a mapping from the encoded hole key to list of
      ;; combos
      (labels ((expand (base)
                 (let ((expanded nil))
                   (loop for item in base
                      do (loop for cand in candidates
                            do (when (>= (aif (car (jewel-combo-jewels cand))
                                              it
                                              -1)
                                         (aif (car (jewel-combo-jewels item))
                                              it
                                              -1))
                                 
                                 (push (merge-jewel-combo cand
                                                          item)
                                       expanded))))
                   expanded))
               (flood-fill (iteration base)
                 (loop for combo in base
                    do (push combo 
                             (gethash (jewel-combo-key combo)
                                      full-map nil)))
                 (when (< iteration 7)
                   (flood-fill (1+ iteration)
                               (expand base)))))
        (flood-fill 1 candidates))
      (loop
         for key being the hash-keys of full-map
         for combos being the hash-values of full-map
         do (push combos (gethash (hole-part key)
                                  hole-map nil)))
      hole-map)))


;;; ---------- Search ----------


(defstruct armor-tree
  (left nil)
  (right nil))

(defstruct armor-preliminary
  (tree nil)
  (jewel-sets nil))

(defun cluster-armors (armor-list required-effects)
  (let ((result (make-map)))
    (loop for piece across armor-list
       do (let ((key (encode-armor piece required-effects)))
            (declare (type (unsigned-byte 64) key))
            (push piece (gethash key result nil))))
    result))


(defparameter *satisfy-test-binary* 
  (the (unsigned-byte 64)
       (let ((result (the (unsigned-byte 64) 0)))
         (declare (type (unsigned-byte 64) result))
         (loop for offset from 17 to 62 by 6
            do (setf (ldb (byte 1 offset) result) 1))
         result)))
  
(declaim (inline is-satisfied-skill-key))
(defun is-satisfied-skill-key (key)
  #f3
  (zerop (logand *satisfy-test-binary* key)))
  

(defun filter-arsenal-with-jewels (required-effects preliminary-arsenal)
  #f3
  (let ((jewel-combos (create-jewel-combos required-effects))
        (result nil)
        (inv-req-key (encode-skill-sig (mapcar #`,(- (cadr x1)) required-effects))))
    (format t "filtering started ...~%")
    (format t "preliminary: ~a~%" (hash-table-count preliminary-arsenal))
    (loop 
       for armor-key being the hash-keys of preliminary-arsenal
       for tree being the hash-values of preliminary-arsenal
       do (awhen (loop for item in 
                      (gethash (the (signed-byte 64) (hole-part armor-key))
                               jewel-combos)
                    when (is-satisfied-skill-key
                          (encoded-skill-+ inv-req-key
                                           (encoded-skill-+ armor-key
                                                            (jewel-combo-key (car item)))))
                    collect item)
            (push (make-armor-preliminary :tree tree
                                          :jewel-sets it)
                  result)))
    result))

(defun search-core (required-effects)
  (let* ((arsenal (list *helms* *cuirasses*
                        *gloves* *cuisses*
                        *sabatons*))
         (clustered-arsenal (mapcar #`,(cluster-armors x1
                                                       required-effects)
                                    arsenal))
         ;; Construct the armor tree (without jewels)
         (preliminary-arsenal
          (reduce (lambda (merged current-part)
                    (let ((new-merged (make-map)))
                      (loop 
                         for current-key being the hash-keys of current-part
                         for current-item being the hash-values of current-part
                         do (loop 
                               for merged-key being the hash-keys of merged
                               for merged-item being the hash-values of merged
                               do (let ((new-key (encoded-+ current-key
                                                            merged-key)))
                                    (declare (type (unsigned-byte 64) new-key))
                                    (push (make-armor-tree :left current-item
                                                           :right merged-item)
                                          (gethash new-key new-merged nil)))))
                      new-merged))
                  clustered-arsenal)))
    preliminary-arsenal))

    ;; Filter out the buckets that cannot meet requirement with jewels
    ;; (filter-arsenal-with-jewels required-effects preliminary-arsenal)))


;;; ---------- Debug Utility ----------






















