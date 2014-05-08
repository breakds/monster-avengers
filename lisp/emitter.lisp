;;;; emitter.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;; 
;;;; Description: emitter is a lazy evaluated stream that can be cascaded. 
;;;; Usage: see emitter-test.lisp

(in-package #:breakds.monster-avengers.emitter)

(declaim (inline empty-emitter))
(defun empty-emitter ()
  (lambda (&optional (reset nil)) 
    (declare (ignorable reset))
    nil))

(defun emitter-from-list (input)
  (let ((upper input))
    (lambda (&optional (reset nil))
      (if reset
          (setf upper input)
          (pop upper)))))


(defmacro emitter-mapcar (input (element) &body body)
  (with-gensyms (upper reset)
    `(let ((,upper ,input))
       (lambda (&optional (,reset nil))
         (if ,reset
             (funcall ,upper t)
             (let ((,element (funcall ,upper)))
               (when ,element
                 ,@body)))))))

(defmacro emitter-mapcan (input (element) &body body)
  (with-gensyms (upper sub-emitter reset)
    `(let ((,upper ,input)
           (,sub-emitter (empty-emitter)))
       (lambda (&optional (,reset nil))
         (if ,reset
             (progn
               (funcall ,upper t)
               (setf ,sub-emitter (empty-emitter)))
             (aif (funcall ,sub-emitter)
                  it
                  (let ((,element (funcall ,upper)))
                    (when ,element
                      (setf ,sub-emitter 
                            (progn ,@body))
                      (funcall ,sub-emitter)))))))))

(defun circular-emitter (input)
  (let ((upper input))
    (lambda (&optional (reset nil))
      (if reset
          (setf upper input)
          (aif (pop upper)
               it
               (progn (setf upper input)
                      (pop upper)))))))

(defmacro emitter-merge (input-a input-b (element-a element-b) &body body)
  (with-gensyms (upper-a upper-b reset)
    `(let ((,upper-a ,input-a)
           (,upper-b ,input-b))
       (lambda (&optional (,reset nil))
         (if ,reset
             (progn (funcall ,upper-a t)
                    (funcall ,upper-b t))
             (let ((,element-a (funcall ,upper-a))
                   (,element-b (funcall ,upper-b)))
               (when (and ,element-a ,element-b)
                 ,@body)))))))

  
  
            
          

(declaim (inline reset-emitter))
(defun reset-emitter (emitter)
  (funcall emitter t))

(declaim (inline emit))
(defun emit (emitter)
  (funcall emitter))
