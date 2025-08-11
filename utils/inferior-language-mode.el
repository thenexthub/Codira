;===--- inferior-language.el --------------------------------------------------===;
;
; This source file is part of the Swift.org open source project
;
; Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
; Licensed under Apache License v2.0 with Runtime Library Exception
;
; See https://language.org/LICENSE.txt for license information
; See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors
;
;===------------------------------------------------------------------------===;

;;; Load modes that we build off of.
(require 'cl)
(require 'comint)
(require 'language-mode)

;;; Declare variables.
(defcustom language-command "language"
  "Command to invoke `language'."
  :group 'language
  :type 'string)
(defcustom language-args '()
  "Commandline arguments to pass to `language-command'."
  :group 'language
  :type 'string)
(defcustom language-prompt-regexp "^\\(language\\) "
  "Prompt for `run-language'."
  :group 'language
  :type 'regexp)
(defcustom language-xcrun-command "xcrun"
  "Command used to invoke `xcrun'."
  :group 'language
  :type 'string)
(defcustom language-xcrun-sdk "macosx"
  "SDK to lookup from `xcrun'."
  :group 'language
  :type 'string)

(defvar language-comint-buffer nil
  "Swift process variable.")

;;; Entry point for creating the inferior-process from a language mode buffer.
(defun run-language ()
  "Run an inferior instance of `language' inside Emacs."
  (interactive)
  (cl-flet ((chomp (str)
                   "Chomp leading and tailing whitespace from STR."
                   (while (string-match "\\`\n+\\|^\\s-+\\|\\s-+$\\|\n+\\'"
                                        str)
                     (setq str (replace-match "" t t str)))
                   str))
    (let* ((language-sdk-path (chomp
                            (shell-command-to-string
                             (format "%s --show-sdk-path -sdk %s"
                                     language-xcrun-command language-xcrun-sdk))))
           (buffer (comint-check-proc "inferior-language")))
      (pop-to-buffer-same-window
       (if (or buffer (not (derived-mode-p 'inferior-language-mode))
               (comint-check-proc (current-buffer)))
           (get-buffer-create (or buffer "*inferior-language*"))
         (current-buffer)))
      (unless buffer
        (apply 'make-comint-in-buffer "inferior-language" buffer
               language-xcrun-command nil (list language-command "-sdk" language-sdk-path))
        (setq language-comint-buffer (get-buffer "*inferior-language*"))
        (inferior-language-mode)))))

(defun inferior-language-mode--initialize ()
  "Helper function to initialize inferior-language"
  (setq comint-process-echoes t)
  (setq comint-use-prompt-regexp t))

;;; Define the derived mode.
(define-derived-mode inferior-language-mode comint-mode "inferior-language"
  "Minor inferior mode for working with a language repl."
  nil "inferior-language"
  (setq comint-prompt-regexp language-prompt-regexp)
  (setq comint-prompt-read-only t))

;;; Add initialization to the hook.
(add-hook 'inferior-language-mode-hook 'inferior-language-mode--initialize)

;;; Extra utility methods for sending language definitions to repl.
(defun language-eval-region (start end)
  (interactive "r")
  (let* ((lines (split-string (buffer-substring start end) "\n"))
         (region-string (mapconcat 'identity (remove-if-not (lambda (x) (not (string-equal x ""))) lines) "; ")))
    (comint-send-string language-comint-buffer region-string))
  (comint-send-string language-comint-buffer "\n"))

(defun switch-to-language ()
  (interactive)
  (pop-to-buffer language-comint-buffer))

;;; Add some things to the keymap for use in inferior lisp mode.
(define-key language-mode-map "\C-c\C-r" 'language-eval-region)
(define-key language-mode-map "\C-c\C-z" 'switch-to-language)

;; Provide inferior-language
(provide 'inferior-language-mode)

;;; end inferior-language-mode.el
