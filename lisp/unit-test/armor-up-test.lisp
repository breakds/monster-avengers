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


      

                
          
                  
                    


