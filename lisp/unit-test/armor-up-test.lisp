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


