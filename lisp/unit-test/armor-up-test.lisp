;;;; armor-up-test.lisp
;;;; Unit tests for armor-up.lisp
;;;; Author: BreakDS <breakds@gmail.com>

(in-package #:breakds.monster-avengers.armor-up-test)

(defsuite* (test-all :in root-suite
                     :documentation "unit tests for armor up"))

(deftest (coding-test
          :cases (('(1 1 5) '(10 -10 20))
                  ('(1 3 0) '(-12))))
    (hole-sig skill-sig)
  (let ((n (length skill-sig)))
    (multiple-value-bind (hole-sig-result 
                          skill-sig-result)
        (decode-sig-full (encode-sig hole-sig skill-sig) n)
      (is (equal hole-sig hole-sig-result))
      (is (equal skill-sig skill-sig-result)))))

(deftest decode-skill-sig-at-test ()
  (let* ((hole-sig '(1 1 5))
	 (skill-sig '(10 -10 20))
	 (key (encode-sig hole-sig skill-sig)))
    (is (= (decode-skill-sig-at key 0) 10))
    (is (= (decode-skill-sig-at key 1) -10))
    (is (= (decode-skill-sig-at key 2) 20))))

(deftest (encoded-+-test
          :cases (('(1 1 0) '(10 -10 20) '(0 1 1) '(-5 3 -2) '(1 2 1) '(5 -7 18))
                  ('(5 1 1) '(-12 -14) '(3 0 0) '(2 -5) '(8 1 1) '(-10 -19))))
    (hole-sig-a skill-sig-a hole-sig-b skill-sig-b hole-sig-+ skill-sig-+)
  (is (= (length skill-sig-a) 
         (length skill-sig-b)
         (length skill-sig-+)))
  (let ((n (length skill-sig-a)))
    (multiple-value-bind (hole-sig-reuslt skill-sig-result)
        (decode-sig-full (encoded-+ (encode-sig hole-sig-a skill-sig-a)
                                    (encode-sig hole-sig-b skill-sig-b))
                         n)
      (is (equal hole-sig-reuslt hole-sig-+))
      (is (equal skill-sig-result skill-sig-+)))))

(deftest (encoded-skill-+-test
          :cases (('(1 1 0) '(10 -10 20) '(0 1 1) '(-5 3 -2) '(0 0 0) '(5 -7 18))
                  ('(5 1 1) '(-12 -14) '(3 0 0) '(2 -5) '(0 0 0) '(-10 -19))))
    (hole-sig-a skill-sig-a hole-sig-b skill-sig-b hole-sig-+ skill-sig-+)
  (is (= (length skill-sig-a) 
         (length skill-sig-b)
         (length skill-sig-+)))
  (let ((n (length skill-sig-a)))
    (multiple-value-bind (hole-sig-reuslt skill-sig-result)
        (decode-sig-full (encoded-skill-+ (encode-sig hole-sig-a skill-sig-a)
					  (encode-sig hole-sig-b skill-sig-b))
                         n)
      (is (equal hole-sig-reuslt hole-sig-+))
      (is (equal skill-sig-result skill-sig-+)))))

(deftest (is-satisfied-skill-key-test
          :cases (('(1 2 1) t)
                  ('(0 0 0 0) t)
                  ('(-1 2 12) nil)
                  ('(0 0 0 -5) nil)))
    (skill-sig expected)
  (is (eq (is-satisfied-skill-key(encode-skill-sig skill-sig))
          expected)))

(deftest encode-jewel-if-satisfy-test ()
  (let ((piece (make-jewel :id 0
                           :name "test jewel"
                           :holes 1
                           :effects '((1 -1) (2 3) (3 5) (4 -5)))))
    (is (null (encode-jewel-if-satisfy piece '(5))))
    (is (null (encode-jewel-if-satisfy piece '(1 4))))
    (is (null (encode-jewel-if-satisfy piece '(1 5))))
    (multiple-value-bind (hole-sig-result
                          skill-sig-result)
        (decode-sig-full (encode-jewel-if-satisfy piece '(3 2 1 5)) 4)
      (is (equal hole-sig-result '(1 0 0)))
      (is (equal skill-sig-result '(5 3 -1 0))))))



;;; Hash table related macros test

(defun hash-table-equal (hash-table-obj assoc-list)
  (and (= (hash-table-count hash-table-obj) (length assoc-list))
       (not (loop for item in assoc-list
	       when (not (equal (cadr item)
				(gethash (car item) hash-table-obj)))
	       return t))))

(defun create-hash-table (assoc-list)
  (let ((result (make-hash-table :test #'eq)))
    (loop for item in assoc-list
       do (setf (gethash (car item) result)
		(cadr item)))
    result))

(deftest classify-to-map-test ()
  (let ((integer-list '((1 2 3)
			(7 9)
			(6)
			(1 5)
			(0 0 0 0)))
	(integer-vector #((1 2 3)
			  (7 9)
			  (6)
			  (1 5)
			  (0 0 0 0))))
    (is (hash-table-equal (classify-to-map :in integer-list
					   :key (apply #'+ individual))
			  '((0 ((0 0 0 0)))
			    (16 ((7 9)))
			    (6 ((1 5) (6) (1 2 3))))))
    (is (hash-table-equal (classify-to-map :across integer-vector
					   :key (length individual)
					   :when (> individual-key 1))
			  '((2 ((1 5) (7 9)))
			    (3 ((1 2 3)))
			    (4 ((0 0 0 0))))))))

(deftest merge-maps-test ()
  (let ((small-map (create-hash-table '((3 (1 2 3))
					(2 (0 1))
					(4 (5)))))
	(big-map (create-hash-table '((9 (8))
				      (7 (4 5 6))))))
    (is (hash-table-equal (merge-maps (small-map big-map)
				      :new-key (+ small-map-key 
						  big-map-key)
				      :when (> (+ small-map-key
						  big-map-key)
					       10)
				      :new-obj (append small-map-val
						       big-map-val))
			  '((12 ((1 2 3 8)))
			    (11 ((5 4 5 6) (0 1 8)))
			    (13 ((5 8))))))
    (let ((result (make-hash-table :test #'eq)))
      (merge-maps (small-map big-map)
    		  :to result
    		  :new-key (+ small-map-key 
			      big-map-key)
    		  :when (> new-key 10)
    		  :new-obj (append small-map-val
				   big-map-val))
      (is (hash-table-equal result
    			    '((12 ((1 2 3 8)))
			      (11 ((5 4 5 6) (0 1 8)))
    			      (13 ((5 8)))))))))



















