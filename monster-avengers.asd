;;;; monster-avengers.asd

(asdf:defsystem #:monster-avengers
    :serial t
    :depends-on (#:struct-wrapper
                 #:basicl
                 #:bordeaux-threads
                 #:hunchentoot
                 #:realispic
                 #:sqlite
                 #:stefil)
    :components ((:file "lisp/package")
                 ;; converter
                 (:file "lisp/converter/mh4gu")
                 ;; crawler
                 (:file "lisp/crawler/mh4g-crawler")
                 (:file "lisp/crawler/mh4-crawler")
                 (:file "lisp/crawler/mhp3-crawler")
                 (:file "lisp/crawler/wikia-mh4u-extender")
                 (:file "lisp/crawler/crawler-test")
                 ;; Engine
                 (:file "lisp/engine/eva-list")
                 (:file "lisp/engine/emitter")
		 (:file "lisp/engine/data-struct")
		 (:file "lisp/engine/data-loader")
		 (:file "lisp/engine/encoding")
		 (:file "lisp/engine/jewels")
                 (:file "lisp/engine/armor-up")
                 (:file "lisp/engine/emitter-test")
                 (:file "lisp/engine/armor-up-test")
                 ;; webapp
                 (:file "lisp/ui/simple-web")))
