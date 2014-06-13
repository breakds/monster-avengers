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


(deftest emitter-mapcar-test ()
  (let* ((source (emitter-from-list '(1 2 3 4)))
         ;; sink = '(2 3 4 5)
         (sink (emitter-mapcar source (x)
                 (1+ x))))
    (is (= (emit sink) 2))
    (reset-emitter sink)
    (is (= (emit sink) 2))
    (is (= (emit sink) 3))
    (is (= (emit sink) 4))
    (is (= (emit sink) 5))
    (is (null (emit sink)))))

(deftest emitter-mapcar-omit-test ()
  (let* ((source (emitter-from-list '(1 2 3 4 5 6 7)))
         ;; sink = '(3 5 7)
         (sink (emitter-mapcar source (x)
                 (when (= (mod x 2) 0)
                   (1+ x)))))
    (is (= (emit sink) 3))
    (reset-emitter sink)
    (is (= (emit sink) 3))
    (is (= (emit sink) 5))
    (is (= (emit sink) 7))
    (is (null (emit sink)))))
                               
(deftest emitter-mapcan-test ()
  (let* ((source (emitter-from-list '((1 2) (3 4))))
         ;; sink = '(2 1 4 3)
         (sink (emitter-mapcan source (x)
                 (emitter-from-list (reverse x)))))
    (is (= (emit sink) 2))
    (reset-emitter source)
    ;; reset source does not change current sink sub-emitter
    (is (= (emit sink) 1))
    ;; resetting sink also resets source
    (reset-emitter sink)
    (is (= (emit sink) 2))
    (is (= (emit sink) 1))
    (is (= (emit sink) 4))
    (is (= (emit sink) 3))
    (is (null (emit sink)))))

(deftest emitter-mapcan-omit-test ()
  (let* ((source (emitter-from-list '((1 2) (3 4) (5 6))))
         ;; sink = '(4 3 6 5)
         (sink (emitter-mapcan source (x)
                 (emitter-from-list (when (> (car x) 2) 
                                      (reverse x))))))
    (is (= (emit sink) 4))
    (reset-emitter source)
    ;; reset source does not change current sink sub-emitter
    (is (= (emit sink) 3))
    ;; resetting sink also resets source
    (reset-emitter sink)
    (is (= (emit sink) 4))
    (is (= (emit sink) 3))
    (is (= (emit sink) 6))
    (is (= (emit sink) 5))
    (is (null (emit sink)))))


(deftest circular-emitter-test ()
  (let ((e (circular-emitter '(1 2 3))))
    (is (= (emit e) 1))
    (is (= (emit e) 2))
    (is (= (emit e) 3))
    ;; looping back
    (is (= (emit e) 1))
    (reset-emitter e)
    (is (= (emit e) 1))
    (is (= (emit e) 2))
    (is (= (emit e) 3))
    (is (= (emit e) 1))
    (is (= (emit e) 2))
    (is (= (emit e) 3))))

(deftest emitter-merge-test ()
  (let* ((a (circular-emitter '(1 2 3)))
         (b (emitter-from-list '(1 2 3 4 5)))
         ;; e = '(2 4 6 5 7)
         (e (emitter-merge a b (x y)
             (+ x y))))
    (is (= (emit e) 2))
    (is (= (emit e) 4))
    (is (= (emit e) 6))
    (reset-emitter e)
    (is (= (emit e) 2))
    (is (= (emit e) 4))
    (is (= (emit e) 6))
    (is (= (emit e) 5))
    (is (= (emit e) 7))
    (is (null (emit e)))))


    
                                  
                                  
                                  

    

           
    
           

    
    

