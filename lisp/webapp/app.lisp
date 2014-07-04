;;;; app.lisp

(in-package #:monster-avengers.webapp)

(eval-when (:compile-toplevel :load-toplevel :execute)
  (enable-jsx-reader))


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

(def-widget search-controller ()
    ()
  #jsx(:div ((class-name "topcoat-button-bar full"))
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
				  "search")))))

(def-widget armor-type-switch ()
    ()
  #jsx(:label ((class-name "topcoat-switch"))
	      (:input ((type "checkbox")
		       (class-name "topcoat-switch__input")))
	      (:div ((class-name "armor-type-switch__toggle armor-type-switch__eng")))))



(def-widget side-bar ()
    ()
  #jsx(:div ((style :width "30%"
                    :min-height "1200px"
                    :left "0px"
                    :box-shadow "5px 0px 20px 0px #999"
                    :position "fixed"
                    :z-index 1
                    :float "left"))
            (:div ((class-name "topcoat-navigation-bar"))
                  (:div ((class-name "topcoat-navigation-bar__item two-thirds left"))
                        (:div ((class-name "topcoat-navigation-bar__title center")
			       (style :text-shadow "0 3px 3px #888"))

                              (:span ((style :color "black"
                                             :font-size 28)) "M")
                              (:span ((style :color "#888888"
                                             :font-size 24))
                                     "ONSTER ")
                              (:span ((style :color "black"
                                             :font-size 28)) "A")
                              (:span ((style :color "#888888"
                                             :font-size 24))
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
			    (:armor-type-switch))
                       (:li ((class-name "topcoat-list__item"))
                            "Attack Up")
                       (:li ((class-name "topcoat-list__item"))
                            "Dodge Up")
                       (:li ((class-name "topcoat-list__item"))
                            "Critical Up")
                       (:li ((class-name "topcoat-list__item"))
			    (:search-controller))))))


(def-widget armor-view (armor-name)
    ()
  #jsx(:li ((class-name "block")
            (style :padding "0px"
                   :box-shadow "7px 7px 20px 0px #555"
                   :margin-left "0px"))
           (:label ((style :border-top "1px solid #878e98"
                           :display "block"
                           :color "#fff"
                           :text-shadow "0 1px 1px #000"
                           :padding "12px"
                           :background "-webkit-gradient(linear, 0% 0%, 0% 100%, from(#888888), to(#737271))"
                           :border-bottom "1px solid #33373d"))
                   (:span ((class-name "icomatic"))
                          "arrowright")
                   (+ " " armor-name))))

(def-widget armor-set-view ()
    ()
  #jsx(:ul ((style :min-height "300px"
                   :margin-left "5%"
                   :margin-right "5%"
                   :margin-top "40px"
                   :box-sizing "border-box"
                   :list-style "none"))
           (:armor-view ((armor-name "Helm")))
           (:armor-view ((armor-name "Chest Plate")))
           (:armor-view ((armor-name "Gloves")))
           (:armor-view ((armor-name "Cuisses")))
           (:armor-view ((armor-name "Boots")))))

                      
                  
            


(def-widget armor-list-view ()
    ()
  #jsx(:div ((style :width "65%"
                    :min-height "1200px"
                    :position "relative"
                    :margin-left "30%"
                    :float "left"))
            (:div ((style :float "right"
                          :width "50%"))
                  (:armor-set-view)
                  (:armor-set-view)
                  (:armor-set-view))
            (:div ((style :overflow "hidden"))
                  (:armor-set-view)
                  (:armor-set-view))))

                          




(def-widget app-view ()
    ()
  #jsx(:div ((style :margin "0 auto"))
            (:side-bar)
            (:armor-list-view)))



(def-realispic-app (avenger-tools :title "Monster Hunter's Arsenal"
                                  :port 16383
                                  :css (;;"http://cdnjs.cloudflare.com/ajax/libs/topcoat/0.8.0/css/topcoat-mobile-light.min.css"
					"css/topcoat/topcoat-mobile-light.min.css"
                                        "css/effeckt.css"
					"css/supplement.css"
                                        "css/icomatic/icomatic.css")
                                  :libs ("lib/react/react-0.10.0.js"
					 "lib/jquery/jquery.min.js")
                                  :document-base (merge-pathnames "assets/"
                                                                  (asdf:system-source-directory 'monster-avengers)))
  #jsx(:app-view))

(eval-when (:compile-toplevel :load-toplevel :execute)
  (disable-jsx-reader))
