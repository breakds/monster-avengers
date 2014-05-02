;;;; armor-up.lisp
;;;; Author: BreakDS <breakds@gmail.com>

(in-package #:breakds.monster-avengers.armor-up)

(defun get-file-name (lang file-name)
  (merge-pathnames (format nil "dataset/~a/~a" lang file-name)
                   (asdf:system-source-directory 'monster-avengers)))

;;; ---------- Constants ----------
(defparameter *foundation-search-cut-off* 2
  "When the number of required-effects <= this cut off value, use
  foundation search only.")

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

;;; ---------- Utilities ----------

(declaim (inline make-map))
(defun make-map ()
  "Create a hash table with unsigned 64 keys."
  (make-hash-table :test #'eq))

(declaim (inline enqueue-map))
(defun enqueue-map (obj key hash-map)
  (push obj (gethash key hash-map nil)))

(declaim (inline max-map-key))
(defun max-map-key (hash-map)
  (loop for key being the hash-keys of hash-map
     maximize key))

(declaim (inline heads-of))
(defun heads-of (input-list n)
  (loop 
     for head in input-list
     for i from 0
     until (>= i n)
     collect head))

(defmacro classify-to-map (&key (in nil) (across nil) (key nil) (when nil))
  "This anarchy macro introduces the variable INDIVIDUAL and INDIVIDUAL-KEY."
  (if (or (and in across)
	  (and (null in) (null across)))
      (error "One and only one of :in and :across should be supplied.")
      (with-gensyms (result)
	`(let ((,result (make-map)))
	   (loop for individual ,@(if across
				      (list 'across across)
				      (list 'in in))
	      do (let ((individual-key ,key))
		   ,(if when
			`(when ,when
			   (enqueue-map individual individual-key ,result))
			`(enqueue-map individual individual-key ,result))))
	   ,result))))

(defmacro merge-maps ((map-a map-b) &key new-key new-obj (to nil) (when nil))
  (with-gensyms (merged-map)
    `(,@(if to
	    `(progn)
	    `(let ((,merged-map (make-map)))))
	(loop
	   for ,(symb map-a '-key) being the hash-keys of ,map-a
	   for ,(symb map-a '-val) being the hash-values of ,map-a
	   do (loop 
		 for ,(symb map-b '-key) being the hash-keys of ,map-b
		 for ,(symb map-b '-val) being the hash-values of ,map-b
		 do (let ((new-key ,new-key))
		      ,(if when
			   `(when ,when
			      (enqueue-map ,new-obj new-key
					   ,(if to to merged-map)))
			   `(enqueue-map ,new-obj new-key 
					 ,(if to to merged-map))))))
	,(when (null to) merged-map))))


;;; ---------- Search ----------

(defstruct armor-tree
  ;; An armor tree is extremely unbalanced, and every left child
  ;; always has a height of 1.
  (left nil)
  (right nil))

(defstruct armor-sets-preliminary
  (forest nil)
  (jewel-sets nil)
  (key 0 :type (unsigned-byte 64)))

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
       for forest being the hash-values of preliminary-arsenal
       do (awhen (loop for item in 
                      (gethash (the (signed-byte 64) (hole-part armor-key))
                               jewel-combos)
                    when (is-satisfied-skill-key
                          (encoded-skill-+ inv-req-key
                                           (encoded-skill-+ armor-key
                                                            (jewel-combo-key (car item)))))
                    collect item)
            (push (make-armor-preliminary :forest forest
                                          :jewel-sets it)
                  result)))
    result))


(defun search-foundation (required-effects)
  (let* ((arsenal (list *helms* *cuirasses*
                        *gloves* *cuisses*
                        *sabatons*))
         (clustered-arsenal 
	  (mapcar (lambda (armor-list) 
		    (classify-to-map :across armor-list
				     :key (the (unsigned-byte 64) 
					       (encode-armor individual 
							     required-effects))))
		  arsenal))
         ;; Construct the armor tree (without jewels)
         (preliminary-arsenal
          (reduce (lambda (merged current-part)
		    (merge-maps (current-part merged)
		    		:new-key (the (unsigned-byte 64)
					      (encoded-+ current-part-key
							 merged-key))
		    		:new-obj (make-armor-tree 
					  :left current-part-val
					  :right merged-val)))
                  clustered-arsenal)))
    (loop 
       for key being the hash-keys of preliminary-arsenal
       for forest being the hash-values of preliminary-arsenal
       collect (make-armor-sets-preliminary :forest forest
                                            :key key))))


          

;; (defun jewel-filter-and-split (required-effects n)
;;   (let ((done-effects (heads-of required-effects n))
;;         (target-effect (car (nthcdr n required-effects))))
;;     (loop for prelim in jew

(declaim (inline points-of-skill))
(defun points-of-skill (armor-piece skill-id)
  (aif (assoc skill-id
	      (armor-effects armor-piece))
       (cadr it) 
       0))

;; (defun extra-skill-split (preliminary target-id target-points hole-map)
;;   (labels ((split-iter (forest minimum-points)
;; 	     ;; The parameter FOREST is a little bit misleading, as it
;; 	     ;; can be a list of armor-trees, or a list of armors (the
;; 	     ;; last level). For each armor-tree in FOREST, the left
;; 	     ;; child is always a list of armors, and the right child
;; 	     ;; can be a list of armors or a forest.
;; 	     (if (armor-p (car forest))
;; 		 ;; case 1: last level
;; 		 (classify-to-map :in forest
;; 				  :key (points-of-skill individual
;; 							target-id)
;; 				  :when (>= individual-key minimum-points))
;; 		 ;; case 2: middle levels
;; 		 (let ((result (make-map)))
;; 		   (loop 
;; 		      for tree in forest
;; 		      for left = (classify-to-map :in (armor-tree-left tree)
;; 						  :key (points-of-skill 
;; 							individual))
;; 		      for right-minimum = (- minimum-points 
;; 					     (max-map-key left))
;; 		      for right = (spliter-iter (armor-tree-rigt tree)
;; 						right-minimum)
;; 		      do (merge-maps (left right)
;; 				     :to result
;; 				     :new-key (+ left-key right-key)
;; 				     :when (>= new-key minimum-points)
;; 				     :new-obj (make-armor-tree
;; 					       :left left-val
;; 					       :right right-val)))
;; 		   result))
;;     (let ((prelim-key (the (unsigned-byte 64)
;; 			   (armor-sets-preliminary-key preliminary)))
;; 	  (pos (length inv-req-key)))
;;       (awhen (gethash (the (unsigned-byte 64) 
;; 			   (hole-part prelim-key))
;; 		      hole-map)
;; 	(let* ((valid-jewel-map 
;; 		(classify-if it 
;; 			     #`,(let ((test-key (encoded-skill-+ 
;; 						 inv-req-key
;; 						 (encoded-skill-+ 
;; 						  prelim-key 
;; 						  (jewel-combo-key (car x1))))))
;; 				     (when (is-satisfied-skill-key test-key)
;; 				       (decode-skill-sig-at test-key pos)))))
;; 	       (forest-minimum (- target-points
;; 				  (loop for key being 
;; 				     the hash-keys of valid-jewel-map
;; 				     maximize key)))
;; 	       (splitted (split-iter (armor-sets-preliminary-forest preliminary)
;; 				     forest-minimum)))
	  
	  

	     
	 
	   
						  
			  
	     

;; (defun make-extra-skill-stream (input-stream required-effects n)
;;   (let* ((done-effects (heads-of required-effects n))
;;          (target-effect (car (nthcdr n required-effects)))
;; 	 (target-id (car target-effect))
;; 	 (target-points (cadr target-effect)))
;;     (funcall (alambda (buffer input-stream)
;; 	       (if buffer
;; 		   #1l(cons (car buffer)
;; 			    (self (rest buffer)
;; 				  input-stream))
;; 		   (awhen (loop 
;; 			     for pointer = input-stream then (cdr$ pointer)
;; 			     for split = (extra-skill-split (car$ pointer)
;; 							    target-id
;; 							    target-points)
;; 			     until (null pointer)
;; 			     when split
;; 			     return (list split pointer))
;; 		     (self (car it) (cdr$ (cadr it))))))
;; 	     nil
;; 	     input-stream)))
		      
			

		      
		      
               

  

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
    preliminary-arsenal
    nil))
         






;;; ---------- Debug Utility ----------






















