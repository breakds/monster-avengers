;;;; init.lisp

(in-package #:monster-avengers.simple-web)

(eval-when (:compile-toplevel :load-toplevel :execute)
  (defparameter *working-dir* "/tmp/")
  (defparameter *server-binary*
    (merge-pathnames "cpp/build/serve_query"
                     (asdf:system-source-directory 'monster-avengers)))
  (defparameter *dataset-path* 
    (merge-pathnames "dataset/MH4GU/"
                     (asdf:system-source-directory 'monster-avengers))))

(eval-when (:compile-toplevel :load-toplevel :execute)
  (defpsmacro lang-text (&rest lang-text-pairs)
    `((chain (lambda ()
               (case (@ this props language)
                 ,@lang-text-pairs))
             (bind this)))))

;; Defins a global object that can be used in realispic widgets, which
;; stores all the skill systems.
(def-global-code skill-systems
  (with-open-file (in (merge-pathnames "skills.lisp"
                                       *dataset-path*)
                      :direction :input)
    `(array 
      ,@(loop for skill-system in (read in)
           collect `(create :name (create :en ,(getf (getf skill-system :name) :en)
                                          :jp ,(getf (getf skill-system :name) :jp))
                            :id ,(getf skill-system :id)
			    :skills (array ,@(loop for skill in (sort (copy-list (getf skill-system 
                                                                                       :skills))
                                                                      #2`,(< (getf x1 :points)
                                                                             (getf x2 :points)))
						when (> (getf skill :points) 0)
						collect `(create :name 
                                                                 (create :en ,(getf (getf skill :name) :en)
                                                                         :jp ,(getf (getf skill :name) :jp))
								 :points ,(getf skill :points)))))))))




