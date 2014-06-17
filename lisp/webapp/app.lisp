;;;; app.lisp

(in-package #:monster-avengers.webapp)

(eval-when (:compile-toplevel :load-toplevel :execute)
  (enable-jsx-reader))

(def-widget search-controller ()
    ()
  #jsx(:div ()
            (:div ((class-name "topcoat-button-bar"))
                  (:div ((class-name "topcoat-button-bar__item"))
                        (:button ((class-name "topcoat-button-bar__button"))
                                 "Add"))
                  (:div ((class-name "topcoat-button-bar__item"))
                        (:button ((class-name "topcoat-button-bar__button"))
                                 "Search")))
            (:label ((class-name "topcoat-switch topcoat-button-bar__button"))
                    (:input ((type "checkbox")
                             (class-name "topcoat-switch__input")))
                    (:div ((class-name "topcoat-switch__toggle"))))))





(def-widget criteria-list ()
    ()
  #jsx(:div ((class-name "topcoat-list")
             (style :float "top"))
            (:ul ((class-name "topcoat-list__container"))
                 (:li ((class-name "topcoat-list__item"))
                      (:span ((style :display "block"))
                             "Attack Up"))
                 (:li ((class-name "topcoat-list__item")) 
                      "Critical Up")
                 (:li ((class-name "topcoat-list__item")) 
                      "Dodge Up"))))

  
            

(def-widget side-bar ()
    ()
  #jsx(:div ((style :width "30%"
                    :min-height "1200px"
                    :left "0px"
                    :box-shadow "5px 0px 20px 0px #999"
                    :float "left"))
            (:div ((class-name "topcoat-navigation-bar"))
                  (:div ((class-name "topcoat-navigation-bar__item two-thirds left"))
                        (:div ((class-name "topcoat-navigation-bar__title center"))
                              (:span ((style :color "black"
                                             :font-size 24)) "M")
                              (:span ((style :color "#888888"
                                             :font-size 18))
                                     "ONSTER ")
                              (:span ((style :color "black"
                                             :font-size 24)) "A")
                              (:span ((style :color "#888888"
                                             :font-size 18))
                                     "VENGERS")))
                  (:div ((class-name "topcoat-navigation-bar__item third right"))
                        (:button ((class-name "topcoat-icon-button--quiet"))
                                 (:span ((class-name "icomatic")
                                         (style :color "#000055"))
                                        "settings"))))
            (:div ((class-name "topcoat-list"))
                  (:h3 ((class-name "topcoat-list__header")) "Criterias")
                  (:ul ((class-name "topcoat-list__container"))
                       (:li ((class-name "topcoat-list__item"))
                            (:label ((class-name "topcoat-switch"))
                                    (:input ((type "checkbox")
                                             (class-name "topcoat-switch__input")))
                                    (:div ((class-name "topcoat-switch__toggle")))))
                       (:li ((class-name "topcoat-list__item"))
                            "Attack Up")
                       (:li ((class-name "topcoat-list__item"))
                            "Dodge Up")
                       (:li ((class-name "topcoat-list__item"))
                            "Critical Up")
                       (:li ((class-name "topcoat-list__item"))
                            (:div ((class-name "topcoat-button-bar full"))
                                  (:div ((class-name "topcoat-button-bar__item"))
                                        (:button ((class-name "topcoat-button-bar__button full"))
                                                 (:span ((class-name "icomatic"))
                                                        "minus")))
                                  (:div ((class-name "topcoat-button-bar__item"))
                                        (:button ((class-name "topcoat-button-bar__button full"))
                                                 (:span ((class-name "icomatic"))
                                                        "plus")))
                                  (:div ((class-name "topcoat-button-bar__item"))
                                        (:button ((class-name "topcoat-button-bar__button full"))
                                                 (:span ((class-name "icomatic")
                                                         (style :color "#000055"))
                                                        "search")))))))))

                            
                  
            


(def-widget armor-list-view ()
    ()
  #jsx(:div ((style :width "65%"
                    :min-height "1200px"
                    :float "left"))))




(def-widget app-view ()
    ()
  #jsx(:div ((style :margin "0 auto"))
            (:side-bar)
            (:armor-list-view)))



(def-realispic-app (avenger-tools :title "Monster Hunter's Arsenal"
                                  :port 16383
                                  :css ("http://cdnjs.cloudflare.com/ajax/libs/topcoat/0.8.0/css/topcoat-mobile-light.min.css"
                                        "css/effeckt.css"
                                        "css/icomatic/icomatic.css")
                                  :libs ("http://fb.me/react-0.10.0.js"
                                         "http://ajax.googleapis.com/ajax/libs/jquery/2.1.1/jquery.min.js")
                                  :document-base (merge-pathnames "assets/"
                                                                  (asdf:system-source-directory 'monster-avengers)))
  #jsx(:app-view))

(eval-when (:compile-toplevel :load-toplevel :execute)
  (disable-jsx-reader))

