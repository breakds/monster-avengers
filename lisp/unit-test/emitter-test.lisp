;;;; emitter-test.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;;
;;;; Description: unit tests for emitter.lisp.

(in-package #:breakds.monster-avengers.emitter-test)

(defsuite* (test-all :in root-suite
                     :documentation "unit test for emiiter."))

(deftest emitter-from-list-test ()
  (let ((e (emitter-from-list nil)))
    (is (null (emit e)))
    (reset-emitter e)
    (is (null (emit e))))
  (let ((e (emitter-from-list '(1 2 3))))
    (is (= (emit e) 1))
    (is (= (emit e) 2))
    (is (= (emit e) 3))
    (is (null (emit e)))
    (reset-emitter e)
    (is (= (emit e) 1))
    (is (= (emit e) 2))
    (is (= (emit e) 3))
    (is (null (emit e)))))

(deftest emitter-from-test ()
  (let* ((source (emitter-from-list '(1 2 3 4)))
         (sink (emitter-from source (x)
                 (1+ x))))
    (is (= (emit sink) 2))
    (is (= (emit sink) 3))
    (reset-emitter sink)
    (is (= (emit sink) 2))
    (is (= (emit sink) 3))
    (is (= (emit sink) 4))
    (is (= (emit sink) 5))
    (is (null (emit sink)))
    (is (null (emit sink)))))

(deftest cached-emitter-from-test ()
  (let* ((source (emitter-from-list '((1 2) (3 4 5))))
         ;; middle = '((3 1 2) (12 3 4 5)))
         (middle (emitter-from source (x)
                   (cons (apply #'+ x) x)))
         ;; sink = '(3 1 2 12 3 4 5)
         (sink (cached-emitter-from middle (x)
                 (pop x))))
    (is (= (emit sink) 3))
    (is (= (emit sink) 1))
    ;; make sure that middle is also advancing
    (is (equal (emit middle) '(12 3 4 5)))
    (reset-emitter middle)
    ;; only reset middle does not change cache
    (is (= (emit sink) 2))
    ;; but when cache is used up, the above reset take effect
    (is (= (emit sink) 3))
    (is (= (emit sink) 1))
    (is (= (emit sink) 2))
    (is (= (emit sink) 12))
    (is (= (emit sink) 3))
    (is (= (emit sink) 4))
    (is (= (emit sink) 5))
    (is (null (emit sink)))
    (is (null (emit sink)))))
    

           
    
           

    
    

