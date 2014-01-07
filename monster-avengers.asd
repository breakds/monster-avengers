;;;; monster-avengers.asd

(asdf:defsystem #:monster-avengers
    :serial t
    :depends-on (#:struct-wrapper
                 #:basicl
                 #:stefil)
    :components ((:file "lisp/package")
                 (:file "lisp/crawler")))
