;;;; simple-web.lisp

(in-package #:monster-avengers.simple-web)

(eval-when (:compile-toplevel :load-toplevel :execute)
  (enable-jsx-reader))

(def-realispic-app (armor-tools (lang) 
                                :title "Monster Hunter's Arsenal"
                                :port 16384
                                :css ("https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.2/css/bootstrap.min.css")
                                :libs ("http://fb.me/react-0.12.2.min.js"
                                       "https://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.3/jquery.min.js"
                                       "https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.2/js/bootstrap.min.js")
                                :document-base (merge-pathnames "assets/"
                                                                (asdf:system-source-directory 'monster-avengers)))
  #jsx(:app-view ((default-language lang))))


(eval-when (:compile-toplevel :load-toplevel :execute)
  (disable-jsx-reader))
