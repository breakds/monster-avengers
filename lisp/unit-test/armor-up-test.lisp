;;;; armor-up-test.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;;
;;;; Description: unit tests for armor-up.lisp.


(in-package #:breakds.monster-avengers.armor-up-test)

(defsuite* (test-all :in root-suite
                     :documentation "unit tests for armor-up."))

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

(deftest (replace-skill-key-at-test
	  :cases ((0 12)
		  (3 5)
		  (4 -6)))
    (n value)
  (let* ((hole-sig '(1 1 5))
	 (skill-sig '(10 -10 20))
	 (key (replace-skill-key-at
	       (encode-sig hole-sig skill-sig)
	       n value)))
    (is (equal (decode-hole-sig key) hole-sig))
    (is (= (decode-skill-sig-at key n) value))))

(deftest (is-satisfied-skill-key-test
          :cases (('(1 2 1) t)
                  ('(0 0 0 0) t)
                  ('(-1 2 12) nil)
                  ('(0 0 0 -5) nil)))
    (skill-sig expected)
  (is (eq (is-satisfied-skill-key (encode-skill-sig skill-sig)
				  (gen-skill-mask (length skill-sig)))
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
      (is (equal hole-sig-result '(0 0 0)))
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

(defun points-map-equal (p-map-a p-map-b)
  (and (= (second p-map-a) (second p-map-b))
       (every (lambda (x)
                (equal (second x) (getf (first p-map-b) (first x))))
              (group (first p-map-a) 2))))

(deftest classify-to-points-map-test ()
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
    (is (points-map-equal (classify-to-points-map :in integer-list
                                                  :key (apply #'+ individual))
                          '((0 ((0 0 0 0)) 16 ((7 9)) 6 ((1 5) (6) (1 2 3))) 16)))
    (is (points-map-equal (classify-to-points-map :across integer-vector
                                                  :key (length individual)
                                                  :when (> individual-key 1))
        		  '((2 ((1 5) (7 9))
                             3 ((1 2 3))
                             4 ((0 0 0 0))) 4)))))

(deftest merge-points-maps-test ()
  (let ((small-map '((3 (1 2 3)
                      2 (0 1)
                      4 (5)) 4))
	(big-map '((9 (8)
                    7 (4 5 6)) 9)))
    (is (points-map-equal (merge-points-maps (small-map big-map)
                                             :new-key (+ small-map-key 
                                                         big-map-key)
                                             :when (> (+ small-map-key
                                                         big-map-key)
                                                      10)
                                             :new-obj (append small-map-val
                                                              big-map-val))
			  '((12 ((1 2 3 8))
                             11 ((5 4 5 6) (0 1 8))
                             13 ((5 8))) 13)))
    (let ((result (make-points-map)))
      (merge-points-maps (small-map big-map)
                             :to result
                             :new-key (+ small-map-key 
                                         big-map-key)
                             :when (> new-key 10)
                             :new-obj (append small-map-val
                                              big-map-val))
      (is (points-map-equal result
    			    '((12 ((1 2 3 8))
                               11 ((5 4 5 6) (0 1 8))
                               13 ((5 8))) 13))))))

(deftest (jewel-set-*-test
          :cases (((list '(2) '(3) nil) 
                   (list '(2) '(3) nil)
                   (list '(2) '(3) nil '(2 3) '(2 2) '(3 3)))
		  ((list nil) (list nil) (list nil))
		  ((list '(1) '(2) '(3) '(4))
		   (list '(5) '(6) '(7) '(8))
		   (list '(:prod ((1) (2) (3) (4)) ((5) (6) (7) (8)))))
                  ((list '(:prod (4) (5)))
                   (list '(2) '(3) nil)
		   ;; note that result won't contain (:prod (4) (5))
		   ;; since empty (nil) only post-connects empty.
                   (list '(:prod (:prod (4) (5)) (2))
                         '(:prod (:prod (4) (5)) (3))))))
    (set-a set-b expected)
  (let ((*jewel-product-calculation-cut-off* 10))
    (is (set-equal (jewel-set-* set-a set-b)
		   expected))))

(defun keyed-jewel-set-equal (x y)
  (and (= (keyed-jewel-set-key x)
	  (keyed-jewel-set-key y))
       (set-equal (keyed-jewel-set-set x)
		  (keyed-jewel-set-set y))))


(deftest exec-super-jewel-set-expr-test ()
  (is (set-equal (exec-super-jewel-set-expr
		  (+ (list (make-keyed-jewel-set :key 10 :set '((1 2 3)))
			   (make-keyed-jewel-set :key 7 :set '((8))))
		     (* (list (make-keyed-jewel-set :key 5 :set '((3) (4)))
			      (make-keyed-jewel-set :key 10 :set '(nil)))
			(list (make-keyed-jewel-set :key 5 :set '((3) (4)))
			      (make-keyed-jewel-set :key 0 :set '((1) (2)))))))
		 (list (make-keyed-jewel-set :key 15 :set '((3) (4)))
		       (make-keyed-jewel-set :key 10 :set '((2) (1) (3 4)
							    (4 4) (3 3)
							    (1 2 3)))
		       (make-keyed-jewel-set :key 7 :set '((8))))
		 :test #'keyed-jewel-set-equal)))

(defparameter *test-jewels* 
  (make-array 
   6
   :initial-contents (list (make-jewel 
			    :id 0
			    :name "Attack I"
			    :holes 1
			    :effects '((0 1) (1 -1)))
			   (make-jewel
			    :id 1
			    :name "Defenese I"
			    :holes 1			    
			    :effects '((0 -1) (1 1)))
			   (make-jewel
			    :id 2
			    :name "Attack II"
			    :holes 2
			    :effects '((0 3) (1 -1)))
			   (make-jewel
			    :id 3
			    :name "Defense II"
			    :holes 2
			    :effects '((0 -1) (1 3)))
			   (make-jewel
			    :id 4
			    :name "Attack III"
			    :holes 3
			    :effects '((0 5) (1 -1)))
			   (make-jewel
			    :id 5
			    :name "Fast III"
			    :holes 3
			    :effects '((2 3))))))

(deftest jewels-encoder-test ()
  (let ((*jewels* *test-jewels*))
    (let ((encoder (jewels-encoder '(2 1 0))))
      (is (equal (decode-skill-sig-full (funcall encoder '(4 3 2)) 3)
                 '(0 1 7)))
      (is (equal (decode-skill-sig-full (funcall encoder '(1 5)) 3)
                 '(3 1 -1))))))


(deftest (stuff-jewels-test 
          :cases (('(1 2 2) '(0 1 2) '(1 0 2))
                  ('(0 0 2) '(2 2) '(2 0 0))
                  ('(0 0 2) '(0 2 5) '(0 0 0))
                  ('(2 2 1) '(0 0 0 0 2 3) '(1 0 0))
                  ('(2 2 1) '(0 0 0 0 3 4) '(0 0 0))))
    (alignment jewel-list expected)
  (let ((*jewels* *test-jewels*))
    (is (equal (stuff-jewels alignment jewel-list)
               expected)))
  (let ((*jewels* *test-jewels*))
    (is (equal (stuff-jewels-fast alignment jewel-list)
               expected))))

(deftest encode-jewels-test ()
  (let ((*jewels* *test-jewels*)
        (jewel-list '(1 3 5))
        (ids '(1 0)))
    (is (equal (decode-skill-sig-full (encode-jewels jewel-list ids) 2)
               '(4 -2)))))
                                      

(deftest (dfs-jewel-query-test
	  :cases (('(0 0 0) '((:key (0 0) :set (nil))))
		  ('(1 0 0) '((:key (0 0) :set (nil))
			      (:key (1 -1) :set ((0)))
			      (:key (-1 1) :set ((1)))))
		  ('(2 0 0) '((:key (0 0) :set (nil (0 1)))
			      (:key (1 -1) :set ((0)))
			      (:key (-1 1) :set ((1)))
			      (:key (-2 2) :set ((1 1)))
			      (:key (2 -2) :set ((0 0)))))
		  ('(1 1 0) '((:key (0 0) :set (nil (0 1)))
		  	      (:key (1 -1) :set ((0) (0 0 1)))
		  	      (:key (-1 1) :set ((1) (0 1 1)))
		  	      (:key (3 -3) :set ((0 0 0)))
		  	      (:key (-3 3) :set ((1 1 1)))
		  	      (:key (2 -2) :set ((0 0)))
		  	      (:key (-2 2) :set ((1 1)))
		  	      (:key (3 -1) :set ((2)))
		  	      (:key (-1 3) :set ((3)))
		  	      (:key (4 -2) :set ((0 2)))
		  	      (:key (0 2) :set ((0 3)))
		  	      (:key (-2 4) :set ((1 3)))
		  	      (:key (2 0) :set ((1 2)))))))
    (hole-alignment expected)
  ;; Assuming there are two skills with id 0 (attack) and 1 (defense).
  (let ((*jewels* *test-jewels*))
    (let ((client (jewel-query-client '((0 2) (1 4)))))
      (is (set-equal (dfs-jewel-query '(0 1) hole-alignment)
		     (loop for item in expected
			collect (make-keyed-jewel-set
				 :key (encode-skill-sig (second item))
				 :set (fourth item)))
		     :test #'keyed-jewel-set-equal)))))


(deftest (jewel-query-client-test 
	  :cases (('(0 0 0))
		  ('(1 0 0))
		  ('(2 0 0))
		  ('(7 0 0))
		  ('(0 1 0))
		  ('(0 5 0))
		  ('(1 1 0))
		  ('(1 2 0))
		  ('(3 4 0))
		  ('(0 0 1))
		  ('(0 1 1))
		  ('(2 2 1))
		  ('(1 2 2))))
    (hole-alignment)
  ;; Assuming there are two skills with id 0 (attack) and 1 (defense).
  (let ((*jewels* *test-jewels*))
    (let* ((required-effects '((0 2) (1 4)))
	   (client (jewel-query-client required-effects)))
      (is (set-equal (funcall client (encode-hole-sig hole-alignment))
		     (dfs-jewel-query (mapcar #'car required-effects) 
				      hole-alignment)
		     :test #'keyed-jewel-set-equal)))))

(deftest (targeted-jewel-query-client-test 
	  :cases (('(0 0 0))
		  ('(1 0 0))
		  ('(2 0 0))
		  ('(7 0 0))
		  ('(0 1 0))
		  ('(0 5 0))
		  ('(1 1 0))
		  ('(1 2 0))
		  ('(3 4 0))
		  ('(0 0 1))
		  ('(0 1 1))
		  ('(2 2 1))
		  ('(1 2 2))))
    (hole-alignment)
  ;; Assuming there are two skills with id 0 (attack) and 1 (defense).
  (let ((*jewels* *test-jewels*))
    (let* ((required-effects '((0 2) (1 4)))
	   (client (jewel-query-client required-effects 1)))
      (is (set-equal (funcall client (encode-hole-sig hole-alignment))
		     (dfs-jewel-query (mapcar #'car required-effects) 
				      hole-alignment 1)
		     :test #'keyed-jewel-set-equal)))))

(defparameter *test-armors*
  (list (make-armor :id 0 :part-id 0 :name "301" :effects '((0 3) (2 1)))
	(make-armor :id 1 :part-id 0 :name "302" :effects '((0 3) (2 2)))
	(make-armor :id 2 :part-id 0 :name "020" :effects '((1 2)))
	(make-armor :id 3 :part-id 0 :name "023" :effects '((1 2) (2 3)))
	(make-armor :id 4 :part-id 1 :name "020" :effects '((1 2)))
	(make-armor :id 5 :part-id 1 :name "021" :effects '((1 2) (2 1)))
	(make-armor :id 6 :part-id 1 :name "400" :effects '((0 4)))
	(make-armor :id 7 :part-id 1 :name "40-2" :effects '((0 4) (2 -2)))
	(make-armor :id 8 :part-id 1 :name "200" :effects '((0 2)))
	(make-armor :id 9 :part-id 2 :name "101" :effects '((0 1) (2 1)))
	(make-armor :id 10 :part-id 2 :name "102" :effects '((0 1) (2 2)))
	(make-armor :id 11 :part-id 2 :name "100" :effects '((0 1)))
	(make-armor :id 12 :part-id 2 :name "003" :effects '((2 3)))
	(make-armor :id 13 :part-id 2 :name "20-1" :effects '((0 2) (2 -1)))))

(defun armor-equal (x y)
  (and (= (armor-id x) (armor-id y))
       (= (armor-part-id x) (armor-part-id y))))

(defun armor-forest-equal (x y)
  (if (armor-p (car y))
      (set-equal x y :test #'armor-equal)
      (set-equal x y
		 :test #'armor-tree-equal)))

(defun armor-tree-equal (x y)
  (and (set-equal (armor-tree-left x)
		  (armor-tree-left y)
		  :test #'armor-equal)
       (armor-forest-equal (armor-tree-right x)
			   (armor-tree-right y))))

(defun prelim-equal (x y)
  (and (= (preliminary-key x) (preliminary-key y))
       (set-equal (preliminary-jewel-sets x)
        	  (preliminary-jewel-sets y)
                  :test #'set-equal)
       (armor-forest-equal (preliminary-forest x)
			   (preliminary-forest y))))

(defun assemble-forest (code-forest)
  (cond ((eq (car code-forest) :tree)
	 (make-armor-tree 
	  :left (loop for code in (second code-forest)
		   collect (nth code *test-armors*))
	  :right (assemble-forest (third code-forest))))
	((consp (car code-forest))
	 (loop for sub-tree in code-forest
	    collect (assemble-forest sub-tree)))
	(t (loop for code in code-forest
	      collect (nth code *test-armors*)))))

(defun ensure-skill-points (forest skill-id skill-points)
  (if (armor-p (car forest))
      (every #`,(= (points-of-skill x1 skill-id) 
		   skill-points)
	     forest)
      (every (lambda (tree)
	       (let ((current (points-of-skill 
			       (car (armor-tree-left tree))
			       skill-id)))
		 (and (ensure-skill-points 
		       (armor-tree-left tree)
		       skill-id
		       current)
		      (ensure-skill-points 
		       (armor-tree-right tree)
		       skill-id
		       (- skill-points current)))))
	     forest)))

(deftest split-forest-at-skill-test ()
  (let ((forest (assemble-forest '((:tree (0 1) ((:tree (4) (9 10))
						 (:tree (5) (11))))
				   (:tree (2 3) ((:tree (6 7) (12))
						 (:tree (8) (13))))))))
    (is (ensure-skill-points forest 0 4))
    (is (ensure-skill-points forest 1 2))
    (let ((result (split-forest-at-skill forest 2 3)))
      (is (armor-forest-equal 
           (getf (car result) 3)
	   (assemble-forest '((:tree (1) ((:tree (4) (9))
					  (:tree (5) (11))))
			      (:tree (0) ((:tree (4) (10))))
			      (:tree (2) ((:tree (6) (12))))))))
      (is (armor-forest-equal 
           (getf (car result) 4)
	   (assemble-forest '((:tree (1) ((:tree (4) (10))))
			      (:tree (3) ((:tree (7) (12))))))))
      (is (armor-forest-equal 
           (getf (car result) 6)
	   (assemble-forest '((:tree (3) ((:tree (6) (12)))))))))))

(defun mock-max-at-skill-client (target-id)
  (let ((store (make-array '(7 400) :element-type 'fixnum :initial-element 0)))
    (declare (type (simple-array fixnum (7 400)) store))
    (loop for item in *test-armors*
       do (setf (aref store (armor-part-id item) (armor-id item))
                (the fixnum (points-of-skill item target-id))))
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



(deftest extra-skill-split-test ()
  (let* ((*jewels* *test-jewels*)
	 (forest (assemble-forest '((:tree (0 1) ((:tree (4) (9 10))
						  (:tree (5) (11))))
				    (:tree (2 3) ((:tree (6 7) (12))
						  (:tree (8) (13)))))))
	 (env (make-split-env :hole-query 
			      (jewel-query-client '((0 6) (1 4) (2 7)))
			      :target-id 2
                              :encoder (jewels-encoder '(0 1 2))
			      :target-points 7
			      :inv-req-key (encode-skill-sig '(-6 -4 -7))
			      :satisfy-mask (gen-skill-mask 3)
                              :forest-maximizer (mock-max-at-skill-client 2)
			      :n 2))
	 (result (extra-skill-split 
		  (make-preliminary :key (encode-sig '(0 2 1)
						     '(4 2))
				    :forest forest
				    :jewel-sets '((2 3)))
		  env)))
    (is (set-equal result
		   (list (make-preliminary 
			  :key (encode-sig '(0 2 1) '(4 2 6))
			  :forest (assemble-forest 
				   '((:tree (3) ((:tree (6) (12))))))
			  :jewel-sets '((2 3 5)))
			 (make-preliminary 
			  :key (encode-sig '(0 2 1) '(4 2 4))
			  :forest (assemble-forest 
				   '((:tree (1) ((:tree (4) (10))))
				     (:tree (3) ((:tree (7) (12))))))
			  :jewel-sets '((2 3 5))))
		   :test #'prelim-equal))))
			 
    
  



















