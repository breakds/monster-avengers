;;;; monster-avengers.asd

(asdf:defsystem #:monster-avengers
    :serial t
    :depends-on (#:struct-wrapper
                 #:basicl
                 #:bordeaux-threads
                 #:stefil)
    :components ((:file "lisp/package")
                 (:file "lisp/crawler")
                 (:file "lisp/eva-list")
                 (:file "lisp/emitter")
		 (:file "lisp/data-struct")
		 (:file "lisp/data-loader")
		 (:file "lisp/encoding")
		 (:file "lisp/jewels")
                 (:file "lisp/armor-up")
                 (:file "lisp/unit-test/crawler-test")
                 (:file "lisp/unit-test/emitter-test")
                 (:file "lisp/unit-test/armor-up-test")))
