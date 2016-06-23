;;; Disable welcome screen
(setq inhibit-startup-screen t)

;;; Load package repositories
(require 'package) 
(add-to-list 'package-archives '("marmalade" . "http://marmalade-repo.org/packages/"))
(add-to-list 'package-archives '("melpa" . "http://melpa.milkbox.net/packages/") t)
(package-initialize)

;;; Remove toolbars
(tool-bar-mode -1)
(menu-bar-mode -1)

;;; Add a dark theme
(setq custom-safe-themes t)
(load-theme 'odersky)

;;; Disable backup
(setq backup-inhibited t)

;;; Disable auto save
(setq auto-save-default nil)

;;; Define org-mode's executable languages 
(org-babel-do-load-languages
 'org-babel-load-languages
 '((sh . t)
   (C . t)
   (haskell . t)
   ))
(setq org-src-fontify-natively t)

;;; Make bold text appear red
(add-to-list 'org-emphasis-alist
	     '("*" (:foreground "yellow1")
	       ))
(add-to-list 'org-emphasis-alist
	     '("/" (:foreground "goldenrod")
	       ))
(add-to-list 'org-emphasis-alist
	     '("_" (:foreground "DarkOliveGreen1")
	       ))
(add-to-list 'org-emphasis-alist
	     '("~" (:foreground "khaki1")
	       ))



;;; Shortcuts for moving betweeen visible buffers
(global-set-key (kbd "C-x C-i") 'windmove-up)
(global-set-key (kbd "C-x C-k") 'windmove-down)
(global-set-key (kbd "C-x C-j") 'windmove-left)
(global-set-key (kbd "C-x C-l") 'windmove-right)

;;; Enable linum mode in C buffers
(add-hook 'c-mode-hook 'linum-mode)
(add-hook 'c++-mode-hook 'linum-mode)



;;; Configuration of irony mode
(add-hook 'c++-mode-hook 'irony-mode)
(add-hook 'c-mode-hook 'irony-mode)
(defun my-irony-mode-hook ()
  (define-key irony-mode-map [remap completion-at-point]
    'irony-completion-at-point-async)
  (define-key irony-mode-map [remap complete-symbol]
    'irony-completion-at-point-async))
(add-hook 'irony-mode-hook 'my-irony-mode-hook)
(add-hook 'irony-mode-hook 'irony-cdb-autosetup-compile-options)



;;; Enable company-mode and make company-irony mode one of its back-ends
(add-hook 'after-init-hook 'global-company-mode)
(setq company-backends '(company-irony))

;;; Enable Flycheck and make flycheck-irony mode one of its c-cpp checker
(add-hook 'after-init-hook 'global-flycheck-mode)
(eval-after-load 'flycheck
  '(add-hook 'flycheck-mode-hook 'flycheck-irony-setup))

;;; Enable Yasnippet
(yas-global-mode 1)
(add-to-list 'yas-snippet-dirs "/home/kisp/yasnippet-snippets")


;;; Enable Autopair in all buffers
(autopair-global-mode) 


;;; Buffer move keybindings
(global-set-key (kbd "<C-up>")     'buf-move-up)
(global-set-key (kbd "<C-down>")   'buf-move-down)
(global-set-key (kbd "<C-left>")   'buf-move-left)
(global-set-key (kbd "<C-right>")  'buf-move-right)


;;; org to latex export settings
(add-to-list 'org-latex-packages-alist '("" "minted"))
(add-to-list 'org-latex-packages-alist '("" "tabularx"))
;; Tell the latex export to use the minted package for source
;; code coloration.
(setq org-latex-listings 'minted)
;; Let the exporter use the -shell-escape option to let latex
;; execute external programs.
;; This obviously and can be dangerous to activate!
(setq org-latex-pdf-process
             '("pdflatex -shell-escape -interaction nonstopmode -output-directory %o %f"
	       "bibtex %b"
	       "pdflatex -shell-escape -interaction nonstopmode -output-directory %o %f"
               "pdflatex -shell-escape -interaction nonstopmode -output-directory %o %f"))
(setq org-latex-minted-options
            '(("frame" "lines") ("linenos=true") ("breaklines=true")))








;;Lisp Environment
					;(setq slime-contribs '(slime-fancy))
					;(setq inferior-lisp-program "/usr/bin/sbcl")
					;(setq emerge-diff-options "--ignore-all-space")






;;(add-hook 'ess-mode-hook
;;	  (lambda () (flycheck-mode t)))





					;(setq user-full-name "Konstantinos Sotiropoulos")
					;(setq user-mail-address "kisp@kth.se")


(setq c-default-style "linux" c-basic-offset 4)
(setq c++-default-style "linux" c++-basic-offset 4)

					;(require 'semantic/bovine/gcc)
					;(semantic-mode 1)
					;(defun my:add-semantic-to-autocomplete()
					;  (add-to-list 'ac-sources 'ac-source-semantic)
					;)
					;(add-hook 'c-mode-common-hook 'my:add-semantic-to-autocomplete)
					;(add-hook 'c++-mode-hook 'my:add-semantic-to-autocomplete)

					;(global-ede-mode 1)

					;(require 'yasnippet)
					;(yas-global-mode 1)






					;(add-hook 'haskell-mode-hook 'haskell-indentation-mode)
					;(add-hook 'after-init-hook 'global-flycheck-mode)
					;(add-hook 'cuda-mode-hook 'linum-mode)
					;(add-hook 'cuda-mode-hook 'autopair-mode)
					;(add-hook 'c++-mode-hook 'autopair-mode)
					;(add-hook 'c++-mode-hook 'linum-mode)



;;(require 'multiple-cursors)
;;(global-set-key (kbd "C-S-c C-S-c") 'mc/edit-lines)




					;R Shit
					;(require 'ess-site)
					;(setq ess-use-auto-complete 'script-only)
					;(autoload 'R-mode "ess-site.el" "" t)
					;(add-to-list 'auto-mode-alist '("\\.R\\'" . R-mode))









;; Include the latex-exporter


					;(require 'org-bibtex)
					;(setq org-bibtex-file "/home/kostis/papers.org")


					;(require 'ox-bibtex)



					;(require 'ox-latex)
;; Add minted to the defaults packages to include when exporting.
					;(add-to-list 'org-latex-packages-alist '("" "minted"))
					;(add-to-list 'org-latex-packages-alist '("" "tabularx"))
;; Tell the latex export to use the minted package for source
;; code coloration.
					;(setq org-latex-listings 'minted)
;; Let the exporter use the -shell-escape option to let latex
;; execute external programs.
;; This obviously and can be dangerous to activate!
					;(setq org-latex-pdf-process
					;      '("pdflatex -shell-escape -interaction nonstopmode -output-directory %o %f"
					;	"bibtex %b" "texi2dvi --pdf --clean --verbose --batch %f"
					;        "pdflatex -shell-escape -interaction nonstopmode -output-directory %o %f"))
					; (setq org-latex-minted-options
					;     '(("frame" "lines") ("linenos=true") ("breaklines=true")))






