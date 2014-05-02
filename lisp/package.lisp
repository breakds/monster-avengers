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

;; Eventually Available List
(defpackage #:breakds.monster-avengers.eva-list
  (:nicknames #:eva-list)
  (:use #:cl
        #:swiss-knife
        #:katana)
  (:export make-eva-list))

(defpackage #:breakds.monster-avengers.armor-up
  (:nicknames #:armor-up)
  (:use #:cl
	#:swiss-knife
	#:struct-wrapper
        #:eva-list)
  (:export #:*jewel-product-calculation-cut-off*
           #:is-satisfied-skill-key
	   ;; Utilities
	   #:classify-to-map
	   #:merge-maps
	   #:individual
	   #:individual-key
	   #:new-key
	   ;; Jewels 
	   #:make-keyed-jewel-set
	   #:keyed-jewel-set-key
	   #:keyed-jewel-set-set
           #:jewel-set-*
	   #:jewel-query-client
	   #:dfs-jewel-query
	   #:exec-super-jewel-set-expr
	   ;; Encoding
           #:encode-hole-sig
           #:decode-hole-sig
           #:encode-skill-sig
           #:decode-skill-sig-full
	   #:decode-skill-sig-at
           #:encode-sig
           #:decode-sig
           #:decode-sig-full
           #:encoded-+
           #:encoded-skill-+
           #:encode-jewel-if-satisfy
	   ;; Data
           #:make-jewel
	   #:*jewels*
	   #:init))

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
        
