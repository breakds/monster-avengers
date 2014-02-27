;;;; package.lisp

(defpackage #:breakds.monster-avengers.crawler
  (:nicknames #:mh4-crawler)
  (:use #:cl
        #:swiss-knife
        #:breakds.html-operation
        #:breakds.struct-wrapper)
  (:export #:crawl-skill-system
           #:crawl-skill-systems
           #:update-jap-dataset))

(defpackage #:breakds.monster-avengers.armor-up
  (:nicknames #:armor-up)
  (:use #:cl
	#:swiss-knife
	#:struct-wrapper)
  (:export #:init
           ;; struct exposure
           #:make-jewel
           ;; encoding system
           #:encode-hole-sig
           #:decode-hole-sig
           #:encode-skill-sig
           #:decode-skill-sig-full
           #:encode-sig
           #:decode-sig
           #:decode-sig-full
           #:encoded-+
           #:encode-jewel-if-satisfy))


;;; ---------- Unit Tests ----------

(defpackage #:breakds.monster-avengers.crawler-test
  (:nicknames #:mh4-crawler-test)
  (:use #:cl
        #:stefil
        #:swiss-knife
        #:breakds.html-operation
        #:breakds.struct-wrapper
        #:breakds.monster-avengers.crawler)
  (:export #:test-all))

(defpackage #:breakds.monster-avengers.armor-up-test
  (:nicknames #:armor-up-test)
  (:use #:cl
        #:stefil
        #:swiss-knife
        #:breakds.struct-wrapper
        #:breakds.monster-avengers.armor-up)
  (:export #:test-all))
        
