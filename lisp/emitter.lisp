;;;; emitter.lisp
;;;; Author: BreakDS <breakds@gmail.com>
;;;; 
;;;; Description: emitter is a lazy evaluated stream that can be cascaded. 
;;;; Usage: see emitter-test.lisp

(in-package #:breakds.monster-avengers.emitter)

(defun emitter-from-list (input)
  (let ((upper input))
    (lambda (&optional (reset nil))
      (if reset
          (setf upper input)
          (pop upper)))))

(defmacro emitter-from (input-emitter (upper-element) &body body)
  (with-gensyms (upper reset)
    `(let ((,upper ,input-emitter))
       (lambda (&optional (,reset nil))
         (if ,reset
             (funcall ,upper t)
             (let ((,upper-element (funcall ,upper)))
               (when ,upper-element
                 ,@body)))))))

(defmacro cached-emitter-from (input (cache) &body body)
  "A cached emitter will cache the result from its upper level and
  keep consuming it. It will refresh the cache if and only if the
  CACHE is set to nil."
  (with-gensyms (upper reset)
    `(let ((,upper ,input)
           (,cache nil))
       (lambda (&optional (,reset nil))
         (if ,reset
            (progn (setf ,cache nil)
                   (funcall ,upper t))
            (progn (when (null ,cache)
                     (setf ,cache (funcall ,upper)))
                   (when ,cache
                     ,@body)))))))

(declaim (inline reset-emitter))
(defun reset-emitter (emitter)
  (funcall emitter t))

(declaim (inline emit))
(defun emit (emitter)
  (funcall emitter))
                     
                    
                    

               
         
             
             
       
           
       
  



              
