;;;; eva-list.lisp
;;;; Author: BreakDS <breakds@gmail.com>

(in-package #:breakds.monster-avengers.eva-list)

(defstruct eva-list
  (generator nil)
  (current nil)
  (index 0 :type (signed-byte 32))
  (filled -1 :type (signed-byte 32))
  (thread nil))

(defun create-eva-list (generator)
  (let ((result (make-eva-list :generator generator)))
    (setf (eva-list-current result) (eva-list-generator result))
    (setf (eva-list-thread result) 
          (bordeaux-threads:make-thread (lambda ()
                                          (format t "haha~%")
                                          (loop for pointer = (eva-list-generator result) then (cdr$ pointer)
                                             until (null pointer)
                                             do (incf (eva-list-filled result)))
                                          (incf (eva-list-filled result) 2))))

    result))

(defun next-ele (eva-list-obj)
  (loop for i = (eva-list-index eva-list-obj)
     until (< i (eva-list-filled eva-list-obj))
     do (sleep 0.01))
  (let ((current (eva-list-current eva-list-obj)))
    (when current
      (incf (eva-list-index eva-list-obj))
      (setf (eva-list-current eva-list-obj) 
            (cdr$ current))
      (car$ current))))



  
  

    
    
                                    
    


