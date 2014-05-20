;;;; jewels.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;; Description: Jewels related operations component of package
;;;; armor-up.

(in-package #:breakds.monster-avengers.armor-up)

(defvar *jewel-product-calculation-cut-off* 20
  "When the length of generated jewel-product >= this cut off,
  generate 'lazy' product instead.")

;;; ---------- Coding Utilities ----------
(declaim (inline jewel-skill-sig))
(defun jewel-skill-sig (jewel-piece required-skill-ids)
  (mapcar #`,(aif (cadr (assoc x1 (jewel-effects jewel-piece)))
                  it
                  0)
          required-skill-ids))

(declaim (inline encode-jewel-if-satisfy))
(defun encode-jewel-if-satisfy (jewel-piece required-skill-ids &optional (n nil))
  "Return the encoded key of a jewel if it has a positive points for
  at least one of the required skills. Return nil otherwise."
  (let ((skill-sig (jewel-skill-sig jewel-piece required-skill-ids)))
    (when (if n 
	      (> (nth n skill-sig) 0)
	      (> (count-if #`,(> x1 0) skill-sig) 0))
      (encode-skill-sig skill-sig))))

;;; ---------- Jewel Search Subroutines ----------

;;; We need some terminologies here to help document the logic here.
;;;
;;; 1. A jewel-list is a list of jewel-ids, e.g. '(1 2) '() '(3
;;; 3 4). A valid jewel-list must have all its element ORDERED. That
;;; says, '(2 1) is not a valid jewel-list.
;;;
;;; 2. A jewel-set is a set of jewel-list that shares the same
;;; signatures. An element in a jewel-set can be a jewel-list, or a
;;; :prod object that represents the product of two jewel-sets.
;;;
;;; 3. A keyed-jewel-set is a pair of key and jewel-set.
;;;
;;; 4. A super-jewel-set is a list of keyed-jewel-set that share the
;;; same hole-signature

(defstruct keyed-jewel-set
  (key 0 :type (unsigned-byte 64))
  (set nil))

(declaim (inline is-prod))
(defun is-prod (element)
  (eq (car element) :prod))

(defun jewel-set-* (jewel-set-a jewel-set-b)
  (if (> (* (length jewel-set-a)
            (length jewel-set-b))
         *jewel-product-calculation-cut-off*)
      (list (list :prod jewel-set-a jewel-set-b))
      (let (result)
        (loop for element-a in jewel-set-a
           do (loop for element-b in jewel-set-b
                 do (cond 
		      ;; 1. empty (nil) pre-connects anything.
		      ((null element-a) (push element-b result))
		      ;; 2. empty (nil) only post-connects empty,
		      ;; which should already be included in case 1.
		      ((null element-b) nil)
		      ;; 3. One of a and b is a prod and the other is
		      ;; not empty, the product is a prod
		      ((or (is-prod element-a) (is-prod element-b))
		       (push (list :prod element-a element-b)
			     result))
		      ;; 4. otherwise, coonect them.
		      (t (when (<= (car (last element-a))
				   (car element-b))
			   (push (append element-a
					 element-b)
				 result))))))
        result)))

(defun compact-super-jewel-set (super-jewel-set)
  (labels ((compact (sorted-result accu)
	     (if (null (cdr sorted-result))
		 (cons (car sorted-result) accu)
		 (let ((a (car sorted-result))
		       (b (cadr sorted-result)))
		   (if (= (keyed-jewel-set-key a)
			  (keyed-jewel-set-key b))
		       (compact (cons (make-keyed-jewel-set
				       :key (keyed-jewel-set-key a)
				       :set (append (keyed-jewel-set-set a)
						    (keyed-jewel-set-set b)))
				      (cddr sorted-result))
				accu)
		       (compact (cdr sorted-result)
				(cons a accu)))))))
    (when super-jewel-set
      (compact (sort (remove-if-not #'keyed-jewel-set-set super-jewel-set)
		     #'< :key #'keyed-jewel-set-key)
	       nil))))


(defun super-jewel-set-* (super-set-a super-set-b)
  (let ((result nil))
    (loop for keyed-set-a in super-set-a
       do (loop for keyed-set-b in super-set-b
	     do (push 
		 (make-keyed-jewel-set 
		  :key (encoded-+ (keyed-jewel-set-key keyed-set-a)
				  (keyed-jewel-set-key keyed-set-b))
		  :set (jewel-set-* (keyed-jewel-set-set keyed-set-a)
				    (keyed-jewel-set-set keyed-set-b)))
		 result)))
    ;; Sort and merge the result with the same key
    (compact-super-jewel-set result)))

(defmacro exec-super-jewel-set-expr (expr)
  "EXPR is an expression with compound + or * (both operator are
  described in /doc/jewel-query/jewel-query.pdf"
  (labels ((expand (expr)
	     (aif (cond ((not (consp expr)) nil)
			 ((eq (car expr) '+) `(append ,(expand (cadr expr))
						      ,(expand (caddr expr))))
			 ((eq (car expr) '*) `(super-jewel-set-* 
					       ,(expand (cadr expr))
					       ,(expand (caddr expr))))
			 (t nil))
		  it
		  expr)))
    `(compact-super-jewel-set ,(expand expr))))

(defmacro mnemonic-alambda (aug-args &body body)
  "This is an alambda which caches all the output whose input has been
  executed at least once."
  (let ((args (mapcar #'car aug-args))
	(rngs (mapcar #'cadr aug-args)))
    (with-gensyms (cache)
    `(let ((,cache (make-array ',rngs :initial-element nil)))
       (labels ((self ,args
                  #f3
		  (aif (aref ,cache ,@args)
		       it
                       (setf (aref ,cache ,@args)
                             (progn
                               ,@body)))))
	 #'self)))))
  
(defun jewel-query-client (required-effects &optional (n nil))
  "The algorithm for generating the jewel combinations for a given
  hole-alignment is described at docs/jewel-query/jewel-query.pdf. The
  only difference here is that we are generating super jewel set
  instead of jewel combinations to include the key."
  #f3
  (let* ((required-skill-ids (mapcar #`,(car x1) required-effects))
	 (calc-fixed (mnemonic-alambda ((holes 4) (num 50))
		       (if (= num 1)
			   (let ((result (when (= holes 1)
					     (list (make-keyed-jewel-set
						    :key 0
						    :set '(nil))))))
			     (loop for item across *jewels*
				when (= (jewel-holes item) holes)
				do (awhen (encode-jewel-if-satisfy
					   item
					   required-skill-ids n)
				     (push (make-keyed-jewel-set 
					    :key it
					    :set (list (list (jewel-id item))))
					   result)))
			     (compact-super-jewel-set result))
			   (exec-super-jewel-set-expr
			    (* (self holes (1- num))
			       (self holes 1))))))
	 (calc (mnemonic-alambda ((i 30) (j 20) (k 8))
		 (cond ((> k 0) (exec-super-jewel-set-expr
				 (+ (* (self i j 0) (funcall calc-fixed 3 k))
				    (self (1+ i) (1+ j) (1- k)))))
		       ((> j 0) (exec-super-jewel-set-expr
				 (+ (* (self i 0 0) (funcall calc-fixed 2 j))
				    (self (+ i 2) (1- j) 0))))
		       ((> i 0) (funcall calc-fixed 1 i))
		       (t (list (make-keyed-jewel-set :key 0 
						      :set '(nil))))))))
    (lambda (hole-key)
      (apply calc (decode-hole-sig hole-key)))))

(defun stuff-if-fit (alignment holes)
  (labels ((align-+ (alignment-a alignment-b)
             (loop 
                for a in alignment-a
                for b in alignment-b
                collect (+ a b))))
    (awhen (case holes
             (1 (cond ((> (first alignment) 0) '(-1 0 0))
                      ((> (second alignment) 0) '(1 -1 0))
                      ((> (third alignment) 0) '(0 1 -1))
                      (t nil)))
             (2 (cond ((> (second alignment) 0) '(0 -1 0))
                      ((> (third alignment) 0) '(1 0 -1))
                      (t nil)))
             (3 (cond ((> (third alignment) 0) '(0 0 -1))
                      (t nil))))
      (align-+ alignment it))))

(defun stuff-jewels (alignment jewel-list)
  (let ((result alignment))
    (loop 
       for id in jewel-list
       for holes = (jewel-holes (aref *jewels* id))
       do (setf result (stuff-if-fit result holes)))
    result))

(declaim (inline stuff-jewels-fast))
(defun stuff-jewels-fast (alignment jewel-list)
  #f3
  (let ((x1 (first alignment))
        (x2 (second alignment))
        (x3 (third alignment))
        (y1 0)
        (y2 0)
        (y3 0))
    (declare (type (signed-byte 8) x1))
    (declare (type (signed-byte 8) x2))
    (declare (type (signed-byte 8) x3))
    (declare (type (signed-byte 8) y1))
    (declare (type (signed-byte 8) y2))
    (declare (type (signed-byte 8) y3))
    (loop 
       for id in jewel-list
       for holes = (jewel-holes (aref *jewels* id))
       do (case holes
            (1 (incf y1))
            (2 (incf y2))
            (3 (incf y3))))
    (decf x3 y3)
    (if (<= y2 x2) 
        (decf x2 y2)
        (progn (decf y2 x2)
               (setf x2 0)
               (incf x1 y2)
               (decf x3 y2)))
    (if (<= y1 x1)
        (progn (decf x1 y1)
               (setf y1 0))
        (progn (decf y1 x1)
               (setf x1 0)))
    (when (> y1 0)
      (if (<= y1 (ash x2 1))
          (progn (decf x2 (ash (1+ y1) -1))
                 (incf x1 (mod y1 2))
                 (setf y1 0))
          (progn (decf y1 (ash x2 1))
                 (setf x2 0)))
      (when (> y1 0)
        (multiple-value-bind (d r) (floor y1 3)
          (decf x3 d)
          (case r
            (1 (decf x3)
               (incf x2))
            (2 (decf x3)
               (incf x1))))))
    (list x1 x2 x3)))

(defun dfs-jewel-query (required-skill-ids hole-alignment &optional (n nil))
  "This is a naive alternative to the previous algorithm. It was
  orignally implemented for checking the correctness of the previous
  algorithm, but can be used in practice as well."
  (let (result)
    (labels ((dfs (alignment current-key current-set jewels)
	       (push (make-keyed-jewel-set
		      :key current-key
		      :set (list current-set))
		     result)
	       (loop 
		  for rest-jewels = jewels then (cdr rest-jewels)
		  for item = (cadar rest-jewels)
		  for key = (caar rest-jewels)
		  until (null rest-jewels)
		  do (awhen (stuff-if-fit alignment 
					  (jewel-holes item))
		       (dfs it 
			    (encoded-+ key current-key)
			    (cons (jewel-id item)
				  current-set)
			    rest-jewels)))))
      (dfs hole-alignment 0 nil 
	   (reverse 
	    (loop 
	       for item across *jewels*
	       for key = (encode-jewel-if-satisfy item required-skill-ids n)
	       when key
	       collect (list key item)))))
    (compact-super-jewel-set result)))

(defun encode-jewels (jewel-list required-skill-ids)
  (let ((result (the (unsigned-byte 64) 0)))
    (declare (type (unsigned-byte 64) result))
    (loop 
       for id in jewel-list
       for skill-sig = (jewel-skill-sig (aref *jewels* id)
                                        required-skill-ids)
       do (setf result (encoded-skill-+ (the (unsigned-byte 64) result)
                                        (the (unsigned-byte 64)
                                             (encode-skill-sig skill-sig)))))
    result))

(defun jewels-encoder (required-skill-ids)
  #f3
  (let ((table (make-array 200 :element-type '(unsigned-byte 64)
                           :initial-element 0)))
    (declare (type (simple-array (unsigned-byte 64) (200)) table))
    (loop for i below (length *jewels*)
       do (awhen (encode-jewel-if-satisfy (aref *jewels* i) required-skill-ids)
            (setf (aref table i) (the (unsigned-byte 64) it))))
    (lambda (jewel-list)
      (let ((result (the (unsigned-byte 64) 0)))
        (loop for id in jewel-list
           do (setf result (encoded-skill-+ result 
                                            (aref table 
                                                  (the (signed-byte 32) id)))))
        result))))
              
                                        



