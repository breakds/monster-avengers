;;;; simple-web.lisp

(in-package #:monster-avengers.simple-web)

(defpsmacro lang-text (&rest lang-text-pairs)
  `(funcall (lambda (language)
              (case language
                ,@lang-text-pairs))
            (@ this props global language)))

(eval-when (:compile-toplevel :load-toplevel :execute)
  (enable-jsx-reader))

(def-widget parameter-panel (global)
    ()
  #jsx(:div ((class-name "row"))
            (:div ((class-name "col-md-2"))
                  (:div ((class-name "panel panel-default"))
                        (:div ((class-name "panel-heading"))
                              (:h3 ((class-name "panel-title"))
                                   (lang-text ("eng" "Weapon Type")
                                              ("zh" "武器类型"))))
                        (:div ((class-name "panel-body"))
                              (:select ((class-name "form-control"))
                                       (:option ((value "eng")) "English")
                                       (:option ((value "zh")) "中文")))))))
                       

(def-widget title-bar (global callback)
    ()
  #jsx(:div ((class-name "topbar")
             (style :width "auto"
                    :margin "auto"))
            (:div ((class-name "fill row"))
                  (:div ((class-name "col-md-4"))
                        (:h3 () (lang-text ("eng" "MH4G Armor Tool")
                                           ("zh" "怪物猎人4G 配装器"))))
                  (:div ((class-name "col-md-2 col-md-offset-6"))
                        (:select ((class-name "form-control")
                                  (on-click (lambda (e)
                                              (funcall callback
                                                       (@ e target value)))))
                                 (:option ((value "eng")) "English")
                                 (:option ((value "zh")) "中文"))))))

(def-widget app-view ()
    ((state (language "eng"))
     (switch-language (target) 
                      (chain this (set-state (create language target)))))
  #jsx(:div ((style :margin "20px 50px 30px 50px"))
            (:title-bar ((global (@ this state))
                         (callback (@ this switch-language))))
            (:parameter-panel ((global (@ this state))))))



(def-realispic-app (armor-tools :title "Monster Hunter's Arsenal"
                                :port 16384
                                :css ("https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.2/css/bootstrap.min.css")
                                :libs ("http://fb.me/react-0.12.2.min.js"
                                       "https://cdnjs.cloudflare.com/ajax/libs/jquery/2.1.3/jquery.min.js"
                                       "https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.2/js/bootstrap.min.js")
                                :document-base (merge-pathnames "assets/"
                                                                (asdf:system-source-directory 'monster-avengers)))
  #jsx(:app-view))

(eval-when (:compile-toplevel :load-toplevel :execute)
  (disable-jsx-reader))
