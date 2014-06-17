;;;; package.lisp

(defpackage #:breakds.monster-avengers.mh4-crawler
  (:nicknames #:mh4-crawler)
  (:use #:cl
        #:swiss-knife
        #:breakds.html-operation
        #:breakds.struct-wrapper)
  (:export #:crawl-skill-system
           #:crawl-skill-systems
           #:update-jap-dataset))

(defpackage #:breakds.monster-avengers.mhp3-crawler
  (:nicknames #:mhp3-crawler)
  (:use #:cl
        #:swiss-knife
        #:breakds.html-operation
        #:breakds.struct-wrapper)
  (:export #:refresh-mhp3-dataset))


;; Eventually Available List
(defpackage #:breakds.monster-avengers.eva-list
  (:nicknames #:eva-list)
  (:use #:cl
        #:swiss-knife
        #:katana)
  (:export make-eva-list))

;; Emitter
(defpackage #:breakds.monster-avengers.emitter
  (:nicknames #:emitter)
  (:use #:cl
        #:swiss-knife)
  (:export #:emitter-from-list
           #:empty-emitter
           #:emitter-mapcar
           #:emitter-mapcan
           #:circular-emitter
           #:emitter-merge
           #:reset-emitter
           #:emit
           #:emit-n))

;; The Engine (armor-up)
(defpackage #:breakds.monster-avengers.armor-up
  (:nicknames #:armor-up)
  (:use #:cl
	#:swiss-knife
	#:struct-wrapper
        #:emitter)
  (:export #:*jewel-product-calculation-cut-off*
           #:is-satisfied-skill-key
	   #:make-split-env
	   #:split-forest-at-skill
	   #:extra-skill-split
	   ;; Utilities
           #:make-points-map
	   #:classify-to-points-map
	   #:merge-points-maps
	   #:classify-to-map
	   #:merge-maps
	   #:individual
	   #:individual-key
	   #:new-key
	   #:gen-skill-mask
	   #:make-armor-tree
	   #:make-preliminary
	   #:preliminary-key
	   #:preliminary-jewel-sets
	   #:preliminary-forest
	   #:armor-tree-left
	   #:armor-tree-right
	   #:armor-forest-navigate
           #:max-at-skill-client
	   ;; Jewels 
	   #:make-keyed-jewel-set
	   #:keyed-jewel-set-key
	   #:keyed-jewel-set-set
           #:jewel-set-*
	   #:jewel-query-client
	   #:dfs-jewel-query
	   #:exec-super-jewel-set-expr
           #:stuff-jewels
           #:stuff-jewels-fast
           #:encode-jewels
           #:jewels-encoder
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
	   #:replace-skill-key-at
	   ;; Data
	   #:make-armor
	   #:armor-p
	   #:armor-id
	   #:armor-part-id
           #:make-jewel
	   #:points-of-skill
	   #:*jewels*
	   #:init))

;; Webapp
(defpackage #:breakds.monster-avengers.webapp
  (:nicknames #:monster-avengers.webapp)
  (:use #:cl
        #:parenscript
        #:realispic)
  (:export #:monster-arsenal))
  

;;; ---------- Unit Tests ----------

(defpackage #:breakds.monster-avengers.crawler-test
  (:nicknames #:mh4-crawler-test)
  (:use #:cl
        #:stefil
        #:swiss-knife
        #:breakds.html-operation
        #:breakds.struct-wrapper
        #:breakds.monster-avengers.mh4-crawler)
  (:export #:test-all))

(defpackage #:breakds.monster-avengers.emitter-test
  (:nicknames #:emitter-test)
  (:use #:cl
        #:stefil
        #:swiss-knife
        #:breakds.monster-avengers.emitter)
  (:export #:test-all))

(defpackage #:breakds.monster-avengers.armor-up-test
  (:nicknames #:armor-up-test)
  (:use #:cl
        #:stefil
        #:swiss-knife
        #:breakds.struct-wrapper
        #:breakds.monster-avengers.armor-up)
  (:export #:test-all))


        
