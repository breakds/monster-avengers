;;;; armor-up.lisp
;;;; Author: BreakDS <breakds@gmail.com>

(in-package #:breakds.monster-avengers.armor-up)


;;; ---------- Constants ----------
(defparameter *foundation-search-cut-off* 2
  "When the number of required-effects <= this cut off value, use
  foundation search only.")

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


;; See unit-test/armor-up-test.lisp for usage.
(defmacro classify-to-map (&key (in nil) (across nil) (key nil) (when nil))
  "This macro generates the code to put the elements in an array or a
   list into buckets managed by a hashtable (map). This anarchy macro
   introduces the variables INDIVIDUAL and INDIVIDUAL-KEY."
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

;; See unit-test/armor-up-test.lisp for usage
(defmacro merge-maps ((map-a map-b) &key new-key new-obj (to nil) (when nil))
  "This macro generates code to merge two maps. It iterates over the
  pairs from the cartesian product of two maps, merge each pair and
  bucket them in a new map. Note that this anarchy macro introduces
  variable (symb map-a '-key) (symb map-a '-val) (symb map-b
  '-key) (symb map-b '-val), as well as NEW-KEY."
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



;;; POINTS-MAP is a light-weight hashtable, implemented as a
;;; two-elements list, where the first element is an assoc-list that
;;; maps points to its bucket, and the second element is the maximumu
;;; of the points in the points-map.

(declaim (inline make-points-map))
(defun make-points-map ()
  (list nil nil))

(declaim (max-points-map-key))
(defun max-points-map-key (p-map)
  (second p-map))

(declaim (enqueue-points-map))
(defun enqueue-points-map (value key p-map)
  "Push a new key-value pair into the points map. If the bucket with
  the points hasn't been created yet, it will be created with a one
  element bucket (list)."
  ;; Update the maximum points when necessary.
  (when (or (null (second p-map))
            (> key (second p-map)))
    (setf (second p-map) key))
  (push value (getf (first p-map) key nil)))

;; classify-to-points-map is a points-map analogy to
;; classify-to-map. See unit-test/armor-up-test.lisp for sample usage.
(defmacro classify-to-points-map (&key (in nil) (across nil) (key nil) (when nil))
  "This anarchy macro introduces the variable INDIVIDUAL and INDIVIDUAL-KEY."
  (if (or (and in across)
	  (and (null in) (null across)))
      (error "One and only one of :in and :across should be supplied.")
      (with-gensyms (result)
	`(let ((,result (make-points-map)))
	   (loop for individual ,@(if across
				      (list 'across across)
				      (list 'in in))
	      do (let ((individual-key ,key))
		   ,(if when
			`(when ,when
			   (enqueue-points-map individual individual-key ,result))
			`(enqueue-points-map individual individual-key ,result))))
	   ,result))))

;; classify-to-points-map is a points-map analogy to merge-maps. See
;; unit-test/armor-up-test.lisp for sample usage.
(defmacro merge-points-maps ((map-a map-b) &key new-key new-obj (to nil) (when nil))
  "This anarchy macro introduces variable (symb map-a '-key) (symb
  map-a '-val) (symb map-b '-key) (symb map-b '-val) as well as
  NEW-KEY."
  (with-gensyms (merged-map)
    `(,@(if to
	    `(progn)
	    `(let ((,merged-map (make-points-map)))))
	(loop for (,(symb map-a '-key) ,(symb map-a '-val))
             on (first ,map-a) by #'cddr
             do (loop for (,(symb map-b '-key) ,(symb map-b '-val))
                   on (first ,map-b) by #'cddr
                   do (let ((new-key ,new-key))
                        ,(if when
                             `(when ,when
                                (enqueue-points-map ,new-obj new-key
                                             ,(if to to merged-map)))
                             `(enqueue-points-map ,new-obj new-key 
                                                  ,(if to to merged-map))))))
	,(when (null to) merged-map))))

(declaim (inline first-n))
(defun first-n (input-list n)
  "Returns two values, where the first value is a list containing the
  first N elements in INPUT-LIST, and the second value is a list
  containing the rest of INPUT-LIST. If INPUT-LIST is shorter than N
  elements, the first return value is going to be identical to
  INPUT-LIST, and the second return value is NIL."
  (let ((l input-list))
    (values
     (loop 
        for i below n
        for x = (pop l)
        until (null x)
        collect x)
     l)))


;;; ---------- Search ----------

;;; The following code form the core of the search algorithm. It is
;;; well-documented but not detailed documented. To have a better
;;; understanding of the algorithm, refer to the document.
;;; TODO(breakds): Create the search algorithm documentation.



;; Armor tree is the foundamemntal data structure in search
;; algorithm. An armor-tree represents a group of armor sets that
;; share the same signature (holes + skills). For each armor tree, 
;; 
;; 1) The left child is a list of armors of the same part
;; (e.g. cuiress), and all these armors have the same signature.
;; 
;; 2) The right child is a forest of armor-trees that share the the
;; same signature.
;;
;; 3) An armor tree with height 2 represents a group of armor sets in
;; which each armor set has two parts. Eventually we want armor trees
;; with height 5 (all parts) or more (with weapon and stone).
;;
;; Therefore, armor trees are extremely unbalanced, and every left
;; child always has a height of 1.
(defstruct armor-tree
  (left nil)
  (right nil))


;; Preliminary represent a preliminary search result. It is a wrapper
;; to a armor forest. 
(defstruct preliminary
  (forest nil)
  ;; jewel-sets is a list of jewel sets, where each of them can make
  ;; the forest meet the requirement.
  (jewel-sets nil)
  ;; key is the encoded signature of the forest (does not include
  ;; jewels).
  (key 0 :type (unsigned-byte 64)))

(defun search-foundation (required-effects type)
  "Search foundation returns a list of preliminaries without jewels,
  which contains all the possible combinations of armors for each kind
  of signature w.r.t. REQUIRED-EFFECTS. Requirement are not met until
  the next step (jewels-filtering). TYPE can be 'melee' or 'range'."
  (let* ((arsenal (list *helms* *cuirasses*
                        *gloves* *cuisses*
                        *sabatons*))
         (clustered-arsenal 
          ;; Group the armors of the same parts and with the same
          ;; signature.
	  (mapcar (lambda (armor-list) 
		    (classify-to-map :across armor-list
                                     ;; filtering out armors with mismatched type
                                     :when (or (string= (armor-type individual) "both")
                                               (string= (armor-type individual) type))
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
    ;; The above result PRELIMINARY-ARSENAL is a map (hash-table) of
    ;; armor-forest. This final step converts it to a list of
    ;; preliminaries instead.
    (loop 
       for key being the hash-keys of preliminary-arsenal
       for forest being the hash-values of preliminary-arsenal
       collect (make-preliminary :forest forest
				 :key key))))

;; SPLIT-ENV conveys the necessary parameters for an
;; extra-skill-split.
(defstruct split-env
  (hole-query nil)
  (encoder nil)
  (target-id 0)
  (target-points 0)
  (inv-req-key 0)
  (satisfy-mask 0)
  (forest-maximizer nil)
  (n 0))

(defun max-at-skill-client (target-id)
  "This function generates a closure with one argument, which is an
  armor-forest. The closure returns the maximum points of the
  specified skill (TARGET-ID) one armor set can achieve within the
  given forest."
  #f3
  ;; STORE is a cache of armor skill points query.
  (let ((store (make-array '(7 400) :element-type 'fixnum :initial-element 0)))
    (declare (type (simple-array fixnum (7 400)) store))
    (loop for part-list in (list *helms* *cuirasses* *gloves* 
                                 *cuisses* *sabatons*)
       do (loop for item across part-list
             do (setf (aref store (armor-part-id item) (armor-id item))
                      (the fixnum (points-of-skill item target-id)))))
    (labels ((client (forest)
               (if (armor-p (car forest))
                   ;; case 1: last level
                   (the fixnum (loop for item in forest
                                  maximize (aref store 
                                                 (armor-part-id item)
                                                 (armor-id item))))
                   ;; case 2 middle levels
                   (the fixnum 
                        (loop for tree in forest
                           maximize (+ (the fixnum 
                                            (loop for item in (armor-tree-left tree)
                                               maximize (aref store 
                                                              (armor-part-id item)
                                                              (armor-id item))))
                                       (client (armor-tree-right tree))))))))
      #'client)))

(defun split-forest-at-skill (forest target-id minimum)
  "The original FOREST is an armor forest with no respect to the skill
  specified by TARGET-ID. This function returns a list of forest
  derived from the input FOREST, each of them will have almost the
  exact singnature as the input FOREST, with an extra chunk for the
  new skill. All the potential forests with points (at TARGET-ID) less
  than minimum will be ignored."
  ;; The parameter FOREST is a little bit misleading, as it
  ;; can be a list of armor-trees, or a list of armors (the
  ;; last level). For each armor-tree in FOREST, the left
  ;; child is always a list of armors, and the right child
  ;; can be a list of armors or a forest.
  (if (armor-p (car forest))
      ;; case 1: last level
      (classify-to-points-map :in forest
                              :key (points-of-skill individual
                                                    target-id)
                              :when (>= individual-key minimum))
      ;; case 2: middle levels
      (let ((result (make-points-map)))
	(loop 
	   for tree in forest
	   for left = (classify-to-points-map 
                       :in (armor-tree-left tree)
                       :key (points-of-skill 
                             individual
                             target-id))
	   for right-minimum = (- minimum 
				  (max-points-map-key left))
	   for right = (split-forest-at-skill (armor-tree-right tree)
					      target-id
					      right-minimum)
	   do (merge-points-maps (left right)
                                 :to result
                                 :new-key (+ left-key right-key)
                                 :when (>= new-key minimum)
                                 :new-obj (make-armor-tree
                                           :left left-val
                                           :right right-val)))
	result)))

(defun expand-jewel-candidates (prelim env)
  "Try adding jewels voting positively to TARGET-ID (from ENV) to the
  current jewel-sets of PRELIM to form new jewel-set candidates. If
  any of the candidates violate the previous requirements (new jewel
  votes negatively to previous skills and drags their points below the
  requirements), or couldn't satisfy the requirement for the target
  skill, ignore it. This is so far the most time consuming part."  
  #f3
  (let* (result
	 (prelim-key (the (unsigned-byte 64)
			  (preliminary-key prelim)))
         ;; MAX-TARGET-POINTS is the maximum we can get from any armor
         ;; set in the forest. If a jewel set candidate cannot meet
         ;; the requirement at target skill with the help of
         ;; MAX-TARGET-POINTS. it will be ignored.
	 (max-target-points (funcall (split-env-forest-maximizer env)
                                     (preliminary-forest prelim)))
	 (prelim-inv-key (encoded-skill-+ 
			  prelim-key
			  (replace-skill-key-at (the (unsigned-byte 64) 0)
						(split-env-n env)
						max-target-points)
			  (split-env-inv-req-key env)))
	 (prelim-holes (decode-hole-sig (hole-part prelim-key))))
    (declare (type (unsigned-byte 64) prelim-key))
    (loop 
       for base-list in (preliminary-jewel-sets prelim)
       for residual = (stuff-jewels-fast prelim-holes base-list)
       when (not (equal residual '(0 0 0)))
       do (let* ((base-key (funcall (split-env-encoder env)
				    base-list))
		 (cands (funcall (split-env-hole-query env)
				 (encode-hole-sig residual))))
	    (loop 
	       for cand in cands
	       for jewels-key = (encoded-skill-+ base-key
						 (keyed-jewel-set-key cand))
	       when (is-satisfied-skill-key 
		     (encoded-skill-+ 
		      prelim-inv-key
		      jewels-key)
		     (split-env-satisfy-mask env))
	       do (setf result (cons (make-keyed-jewel-set 
	       			      :key jewels-key
	       			      :set (mapcar (lambda (x) 
	       					     (append x base-list))
	       					   (keyed-jewel-set-set cand)))
	       			     result)))))	
    result))

(defun extra-skill-split (prelim env)
  "This is the core of the search algorithm. PRELIM is one of the
  preliminaries that represent a list of armor sets meeting the
  requirements of previous skills. EXTRA-SKILL-SPLIT attempts to
  generate new preliminaries out of PRELIM that meets the requirement
  of the target skill (in ENV) as well."
  (let* ((n (split-env-n env))
	 (target-points (split-env-target-points env))
	 (prelim-key (the (unsigned-byte 64)
			  (preliminary-key prelim)))
	 (jewel-cands (expand-jewel-candidates prelim env))
	 (minimum (- target-points
		     (loop for cand in jewel-cands
			maximize (decode-skill-sig-at 
				  (keyed-jewel-set-key cand)
				  n)))))
    ;; Failing to get any possible jewel candidates means we can not
    ;; derive any preliminaries from PRELIM that satisfy both previous
    ;; skills and target skill requirements.
    (when jewel-cands
      (let ((armor-cands (split-forest-at-skill
                          (preliminary-forest prelim)
                          (split-env-target-id env)
                          minimum)))
        (loop for (points forest) on (first armor-cands) by #'cddr
           ;; for points being the hash-keys of armor-cands
           ;; for forest being the hash-values of armor-cands
           for valid-sets = (loop for item in jewel-cands
                               when (>= (+ points
                                           (decode-skill-sig-at
                                            (keyed-jewel-set-key item)
                                            n))
                                        target-points)
                               append (keyed-jewel-set-set item))
           when valid-sets
           collect (make-preliminary :key (replace-skill-key-at prelim-key
                                                                n
                                                                points)
                                     :forest forest
                                     :jewel-sets valid-sets))))))

(defun make-extra-skill-emitter (input required-effects n)
  (let* ((required-ids (mapcar #`,(first x1) (first-n required-effects (1+ n))))
         (env (make-split-env :hole-query (jewel-query-client 
                                           (map-n #`,(nth x1 required-effects) 
                                                  (1+ n))
                                           n)
                              :encoder (jewels-encoder required-ids)
                              :target-id (first (nth n required-effects))
                              :target-points (second (nth n required-effects))
                              :inv-req-key (encode-skill-sig 
                                            (mapcar #`,(- (second x1)) 
                                                    required-effects))
                              :satisfy-mask (gen-skill-mask (1+ n))
                              :forest-maximizer (max-at-skill-client 
                                                 (first (nth n required-effects)))
                              :n n)))
    (emitter-mapcan input (x)
      (emitter-from-list (extra-skill-split x env)))))

(defun make-jewel-filter-emitter (input required-effects)
  "Filtering out the preliminaries in the INPUT emitter that can not
  meet the REQUIRED-EFFECTS with any jewel sets that fit into its
  holes. For the ones who survived the test, put the jewel sets
  information into it. This function returns a preliminary emitter."
  (let ((hole-query (jewel-query-client required-effects))
        (inv-req-key (encode-skill-sig (mapcar #`,(- (second x1))
                                               required-effects)))
        (satisfy-mask (gen-skill-mask (length required-effects))))
    (emitter-mapcar input (x)
      (let ((prelim-key (preliminary-key x)))
        (awhen (loop for cand in (funcall hole-query prelim-key)
                  when (is-satisfied-skill-key
                        (encoded-skill-+ (keyed-jewel-set-key cand)
                                         inv-req-key
                                         prelim-key)
                        satisfy-mask)
                  append (keyed-jewel-set-set cand))
          (make-preliminary :key prelim-key
                            :forest (preliminary-forest x)
                            :jewel-sets it))))))

(defun search-core (required-effects &optional (type "melee"))
  "This function returns an emitter that emits one preliminary each
  call that satisfies all the REQUIRED-EFFECTS."
  (let* ((foundation-req (first-n required-effects
                                  *foundation-search-cut-off*))
         (foundation (make-jewel-filter-emitter
                      (emitter-from-list
                       (search-foundation foundation-req type))
                      foundation-req)))
    ;; Foundation is a list-emitter generated from the result of
    ;; SEARCH-FOUNDATION. It considers only the first
    ;; *FOUNDATION-SEARCH-CUT-OFF* of REQUIRED-EFFECTS. Other
    ;; requirements are added one by one in the following code by
    ;; applying an extra-skill-emitter upon the previous emitter.
    (reduce (lambda (y x)
              (make-extra-skill-emitter y required-effects x))
            (loop 
               for i from *foundation-search-cut-off* 
               below (length required-effects)
               collect i)
            :initial-value foundation)))

(defun emitter-from-tree (tree)
  (emitter-merge 
      (circular-emitter (armor-tree-left tree))
      (emitter-from-forest (armor-tree-right tree))
      (x y)
    (cons x y)))

(defun emitter-from-forest (forest)
  (if (armor-p (car forest))
      ;; if forest is actually an armor list.
      (emitter-mapcar (emitter-from-list forest) (x)
        (list x))
      ;; otherwise, real forest (list of trees).
      (emitter-mapcan (emitter-from-list forest) (x)
        (emitter-from-tree x))))

(defun make-armor-set-emitter (input)
  "Emitter that emits armor sets from a preliminary emitter. This is
  the last step of the search (to show the search result in a readable
  way)."
  (emitter-mapcan input (x)
    (emitter-mapcar 
        (emitter-from-forest (preliminary-forest x)) 
        (armor-list)
      (list armor-list
            (preliminary-jewel-sets x)
            (preliminary-key x)))))

;;; ---------- Debug Utility ----------

(defmacro armor-forest-navigate (tree &rest nav)
  (reduce (lambda (y x) (case x
			  (< `(armor-tree-left ,y))
			  (> `(armor-tree-right ,y))
			  (t `(nth ,x ,y))))
	  nav
	  :initial-value tree))

(defun stringify-effect (effect)
  (concatenate 'string 
               (get-name (aref *skill-systems* 
                                        (car effect)))
               (format nil "~a~a" 
                       (if (> (second effect) 0) "+" "")
                       (second effect))))

(defun print-armor (item)
  (format t "[~a~a~a] ~a  ~{~a ~}~%" 
          (if (< 0 (armor-holes item)) "O" "-")
          (if (< 1 (armor-holes item)) "O" "-")
          (if (< 2 (armor-holes item)) "O" "-")
          (get-name item)
          (loop for effect in (armor-effects item)
             collect (stringify-effect effect))))

(defun print-jewel (item)
  (format t "([~a~a~a] ~a ~{~a ~})~%"
          (if (< 0 (jewel-holes item)) "O" "x")
          (if (< 1 (jewel-holes item)) "O" "x")
          (if (< 2 (jewel-holes item)) "O" "x")
          (get-name item)
          (loop for effect in (jewel-effects item)
             collect (stringify-effect effect))))

(defun print-armor-set (armor-set)
  (let ((all-effects))
    (labels ((add-armor-effects (item)
               (loop for effect in (armor-effects item)
                  do (if (getf all-effects (first effect))
                         (incf (getf all-effects (first effect))
                               (second effect))
                         (setf (getf all-effects (first effect)) 
                               (second effect)))))
             (add-jewel-effects (item)
               (loop for effect in (jewel-effects item)
                  do (if (getf all-effects (first effect))
                         (incf (getf all-effects (first effect)) 
                               (second effect))
                         (setf (getf all-effects (first effect)) 
                               (second effect))))))
      (loop for item in (first armor-set)
         do 
           (print-armor item)
           (add-armor-effects item))
      (loop for id in (first (second armor-set))
         do 
           (print-jewel (aref *jewels* id))
           (add-jewel-effects (aref *jewels* id)))
      (format t "~{~a~%~}"
              (loop for effect in (group all-effects 2)
                 collect (stringify-effect effect)))
      (decode-sig-full (third armor-set) 3))))
                    
  

