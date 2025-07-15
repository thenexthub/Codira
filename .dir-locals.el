;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

((nil
  (eval let*
        ((x (dir-locals-find-file default-directory))
         (this-directory (if (listp x) (car x) (file-name-directory x))))
        (unless (or (featurep 'language-project-settings) 
                    (and (fboundp 'tramp-tramp-file-p)
                         (tramp-tramp-file-p this-directory)))
          (add-to-list 'load-path
                       (concat this-directory "utils")
                       :append)
          (defvar language-project-directory)
          (let ((language-project-directory this-directory))
            (require 'language-project-settings)))
        (set (make-local-variable 'language-project-directory)
         this-directory)
        )
  (fill-column . 80)
  (c-file-style . "language"))
 (c++-mode
  (whitespace-style face lines indentation:space)
  (flycheck-clang-language-standard . "c++14"))
 (c-mode
  (whitespace-style face lines indentation:space))
 (objc-mode
  (whitespace-style face lines indentation:space))
 (prog-mode
  (eval add-hook 'prog-mode-hook
        (lambda nil
          (whitespace-mode 1))
        (not :APPEND)
        :BUFFER-LOCAL))
 (language-mode
  (language-find-executable-fn . language-project-executable-find)
  (language-syntax-check-fn . language-project-language-syntax-check)
  (whitespace-style face lines indentation:space)
  (language-basic-offset . 2)
  (tab-always-indent . t)))



;; Local Variables:
;; eval: (whitespace-mode -1)
;; End:
