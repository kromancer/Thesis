(setq inhibit-startup-message t)
(menu-bar-mode -1)
(tool-bar-mode -1)
(setq visible-bell 1)

;;;;;;;;;;;;;
;; MELPA 
;;;;;;;;;;;;;
(require 'package)
(setq package-enable-at-startup nil)
(add-to-list 'package-archives
	     '("melpa" . "http://melpa.org/packages/"))
(package-initialize)

;;;;;;;;;;;;;;;;;
;; Color Themes
;;;;;;;;;;;;;;;;;
(use-package sublime-themes
  :ensure t
  :config (load-theme 'hickey t))


;;;;;;;;;;;;;;;;
;; The Z's tips
;;;;;;;;;;;;;;;;
(setq ido-enable-flex-matching t)
(setq ido-everywhere t)
(ido-mode 1)
(defalias 'list-buffers 'ibuffer)
(winner-mode 1)


;;;;;;;;;;;;;;;;;;;;
;; Windows Movement
;;;;;;;;;;;;;;;;;;;;
(use-package ace-window
  :ensure t
  :init
  (progn
    (global-set-key [remap other-window] 'ace-window)
    (custom-set-faces
     '(aw-leading-char-face
       ((t (:inherit ace-jump-face-foreground :height 3.0)))))
))

(global-set-key (kbd "C-x C-i") 'windmove-up)
(global-set-key (kbd "C-x C-k") 'windmove-down)
(global-set-key (kbd "C-x C-j") 'windmove-left)
(global-set-key (kbd "C-x C-l") 'windmove-right)

;;;;;;;;;;;;;;;;;;;;;
;; Ivy-Swiper-Counsel
;;;;;;;;;;;;;;;;;;;;;
(use-package counsel
  :ensure t
)

(use-package ivy
  :ensure t
  :diminish (ivy-mode)
  :bind (("C-x b" . ivy-switch-buffer))
  :config
  (ivy-mode 1)
  (setq ivy-use-virtual-buffers t)
  (setq ivy-display-style 'fancy))

(use-package swiper
  :ensure t
  :bind (("C-s" . swiper)
	 ("C-r" . swiper)
	 ("C-c C-r" . ivy-resume)
	 ("M-x" . counsel-M-x)
	 ("C-x C-f" . counsel-find-file))
  :config
  (progn
    (ivy-mode 1)
    (setq ivy-use-virtual-buffers t)
    (setq ivy-display-style 'fancy)
    (define-key read-expression-map (kbd "C-r") 'counsel-expression-history)
))

(use-package auto-complete
  :ensure t
  :init
  (progn
    (ac-config-default)
    (global-auto-complete-mode t)
))   




(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(custom-safe-themes (quote ("e0d42a58c84161a0744ceab595370cbe290949968ab62273aed6212df0ea94b4" default))))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(default ((t (:background nil))))
 '(aw-leading-char-face ((t (:inherit ace-jump-face-foreground :height 3.0)))))
