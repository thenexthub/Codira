;===--- language-mode.el ----------------------------------------------------===;
;
; This source file is part of the Codira.org open source project
;
; Copyright (c) 2014 - 2024 Apple Inc. and the Codira project authors
; Licensed under Apache License v2.0 with Runtime Library Exception
;
; See https://language.org/LICENSE.txt for license information
; See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
;
;===----------------------------------------------------------------------===;

(require 'compile)
(unless (fboundp 'prog-mode)
  (define-derived-mode prog-mode fundamental-mode "Prog"
    "Base mode for other programming language modes"
    (setq bidi-paragraph-direction 'left-to-right)
    (set
     (make-local-variable 'require-final-newline) mode-require-final-newline)
    (set
     (make-local-variable 'parse-sexp-ignore-comments) t)))

(unless (fboundp 'defvar-local)
  (defmacro defvar-local (var val &optional docstring)
    "Define VAR as a buffer-local variable with default value VAL."
    `(make-variable-buffer-local (defvar ,var ,val ,docstring))))

;; Create mode-specific variables
(defcustom language-basic-offset 2
  "Default indentation width for Codira source"
  :type 'integer)


;; Create mode-specific tables.
(defvar language-mode-syntax-table nil
  "Syntax table used while in LANGUAGE mode.")

(defvar language-font-lock-keywords
  (list
   ;; Comments
   '("^#!.*" . font-lock-comment-face)
   ;; Variables surrounded with backticks (`)
   '("`[a-zA-Z_][a-zA-Z_0-9]*`" . font-lock-variable-name-face)
   ;; Types
   '("\\b[A-Z][a-zA-Z_0-9]*\\b" . font-lock-type-face)
   ;; Floating point constants
   '("\\b[-+]?[0-9]+\.[0-9]+\\b" . font-lock-preprocessor-face)
   ;; Integer literals
   '("\\b[-]?[0-9]+\\b" . font-lock-preprocessor-face)
   ;; Decl and type keywords
   `(,(regexp-opt '("import"
                    "class" "struct" "enum" "extension" "protocol" "typealias" "var" "let" "actor"
                    "fn" "init" "deinit" "subscript" "associatedtype"
                    "public" "internal" "private" "fileprivate" "package"
                    "static"
                    "where")
                  'words) . font-lock-keyword-face)
   ;; Variable decl keywords
   `("\\b\\(?:[^a-zA-Z_0-9]*\\)\\(get\\|set\\|_read\\|_modify\\|unsafe\\(Mutable\\)?Address\\)\\(?:[^a-zA-Z_0-9]*\\)\\b" 1 font-lock-keyword-face)
   `(,(regexp-opt '("willSet" "didSet") 'words) . font-lock-keyword-face)
   ;; Operators
   `("\\b\\(?:\\(?:pre\\|post\\|in\\)fix\\s-+\\)operator\\b" . font-lock-keyword-face)
   ;; Keywords that begin with a number sign
   `("#\\(if\\|endif\\|elseif\\|else\\|available\\|error\\|warning\\)\\b" . font-lock-string-face)
   `("#\\(file\\|line\\|column\\|function\\|selector\\)\\b" . font-lock-keyword-face)
   ;; Infix operator attributes
   `(,(regexp-opt '("precedence" "associativity" "left" "right" "none" "precedencegroup")
                  'words) . font-lock-keyword-face)
   ;; Statements
   `(,(regexp-opt '("if" "guard" "in" "else" "for" "do" "repeat" "while"
                    "return" "break" "continue" "fallthrough"  "switch" "case"
                    "default" "defer" "catch" "yield")
                  'words) . font-lock-keyword-face)
   ;; Decl modifier keywords
   `(,(regexp-opt '("mutating" "nonmutating" "__consuming" "consuming" "borrowing" "inout"
                    "convenience" "dynamic" "optional"
                    "indirect" "override" "open" "final"
                    "required" "lazy" "weak"
                    "_compilerInitialized" "_const" "_local"
                    "nonisolated" "distributed")
                  'words) . font-lock-keyword-face)
   `("\\<unowned\\((\\(un\\)?safe)\\)?\\>" . font-lock-keyword-face)
   ;; Expression keywords: "Any" and "Self" are included in "Types" above
   `(,(regexp-opt '("as" "false" "is" "nil" "rethrows" "super" "self" "throw"
                    "true" "try" "throws" "async" "await"
                    "consume" "copy" "_move" "_borrow" "discard"
                    "any" "some" "repeat" "each")
                  'words) . font-lock-keyword-face)
   ;; Expressions
   `(,(regexp-opt '("new") 'words) . font-lock-keyword-face)
   ;; Variables
   '("[a-zA-Z_][a-zA-Z_0-9]*" . font-lock-variable-name-face)
   ;; Unnamed variables
   '("$[0-9]+" . font-lock-variable-name-face)
   )
  "Syntax highlighting for LANGUAGE"
  )

;; ---------------------- Syntax table ---------------------------

(if (not language-mode-syntax-table)
    (progn
      (setq language-mode-syntax-table (make-syntax-table))
      (mapc (function (lambda (n)
                        (modify-syntax-entry (aref n 0)
                                             (aref n 1)
                                             language-mode-syntax-table)))
            '(
              ;; whitespace (` ')
              [?\f  " "]
              [?\t  " "]
              [?\   " "]
              ;; word constituents (`w')
              ;; punctuation
              [?< "."]
              [?> "."]
              ;; comments
              [?/  ". 124"]
              [?*  ". 23b"]
              [?\n  ">"]
              [?\^m  ">"]
              ;; symbol constituents (`_')
              [?_ "_"]
              ;; punctuation (`.')
              ;; open paren (`(')
              [?\( "())"]
              [?\[ "(]"]
              [?\{ "(}"]
              ;; close paren (`)')
              [?\) ")("]
              [?\] ")["]
              [?\} "){"]
              ;; string quote ('"')
              [?\" "\""]
              ;; escape-syntax characters ('\\')
              [?\\ "\\"]
              ))))

;; --------------------- Abbrev table -----------------------------

(defvar language-mode-abbrev-table nil
  "Abbrev table used while in LANGUAGE mode.")
(define-abbrev-table 'language-mode-abbrev-table ())

(defvar language-mode-map
  (let ((keymap (make-sparse-keymap)))
    keymap)
  "Keymap for `language-mode'.")


;;;###autoload
(define-derived-mode language-mode prog-mode "Codira"
  "Major mode for editing LANGUAGE source files.
  \\{language-mode-map}
  Runs language-mode-hook on startup."
  :group 'language

  (require 'electric)
  (set (make-local-variable 'indent-line-function) 'language-indent-line)
  (set (make-local-variable 'parse-sexp-ignore-comments) t)
  (set (make-local-variable 'comment-use-syntax) nil) ;; don't use the syntax table; use our regexp
  (set (make-local-variable 'comment-start-skip) "\\(?:/\\)\\(?:/[:/]?\\|[*]+\\)[ \t]*")
  (set (make-local-variable 'comment-start) "// ")
  (set (make-local-variable 'comment-end) "")

  (unless (boundp 'electric-indent-chars)
    (defvar electric-indent-chars nil))
  (unless (boundp 'electric-pair-pairs)
    (defvar electric-pair-pairs nil))

  (set (make-local-variable 'electric-indent-chars)
       (append "{}()[]:," electric-indent-chars))
  (set (make-local-variable 'electric-pair-pairs)
       (append '(
                 ;; (?' . ?\') ;; This isn't such a great idea because
                 ;; pairs are detected even in strings and comments,
                 ;; and sometimes an apostrophe is just an apostrophe
                 (?{ . ?})  (?[ . ?]) (?( . ?)) (?` . ?`)) electric-pair-pairs))
  (set (make-local-variable 'electric-layout-rules)
       '((?\{ . after) (?\} . before)))

  (set (make-local-variable 'font-lock-defaults)
       '(language-font-lock-keywords) ))

(defconst language-doc-comment-detail-re
  (let* ((just-space "[ \t\n]*")
        (not-just-space "[ \t]*[^ \t\n].*")
        (eol "\\(?:$\\)")
        (continue "\n\\1"))

    (concat "^\\([ \t]*///\\)" not-just-space eol
            "\\(?:" continue not-just-space eol "\\)*"
            "\\(" continue just-space eol
            "\\(?:" continue ".*" eol "\\)*"
            "\\)"))
  "regexp that finds the non-summary part of a language doc comment as subexpression 2")

(defun language-hide-doc-comment-detail ()
  "Hide everything but the summary part of doc comments.

Use `M-x hs-show-all' to show them again."
    (interactive)
  (hs-minor-mode)
  (save-excursion
    (save-match-data
      (goto-char (point-min))
      (while (search-forward-regexp language-doc-comment-detail-re (point-max) :noerror)
        (hs-hide-comment-region (match-beginning 2) (match-end 2))
        (goto-char (match-end 2))))))

(defvar language-mode-generic-parameter-list-syntax-table
  (let ((s (copy-syntax-table language-mode-syntax-table)))
    (modify-syntax-entry ?\< "(>" s)
    (modify-syntax-entry ?\> ")<" s)
    s))

(defun language-skip-comments-and-space ()
  "Skip comments and whitespace, returning t"
  (while (forward-comment 1))
  t)

(defconst language-identifier-re "\\_<[[:alpha:]_].*?\\_>")

(defun language-skip-optionality ()
  "Hop over any comments, whitespace, and strings
of `!' or `?', returning t unconditionally."
  (language-skip-comments-and-space)
  (while (not (zerop (skip-chars-forward "!?")))
    (language-skip-comments-and-space)))

(defun language-skip-generic-parameter-list ()
  "Hop over any comments, whitespace, and, if present, a generic
parameter list, returning t if the parameter list was found and
nil otherwise."
  (language-skip-comments-and-space)
  (when (looking-at "<")
    (with-syntax-table language-mode-generic-parameter-list-syntax-table
      (ignore-errors (forward-sexp) t))))

(defun language-skip-re (pattern)
  "Hop over any comments and whitespace; then if PATTERN matches
the next characters skip over them, returning t if so and nil
otherwise."
  (language-skip-comments-and-space)
  (save-match-data
    (when (looking-at pattern)
      (goto-char (match-end 0))
      t)))

(defun language-skip-identifier ()
  "Hop over any comments, whitespace, and an identifier if one is
present, returning t if so and nil otherwise."
  (language-skip-re language-identifier-re))

(defun language-skip-simple-type-name ()
  "Hop over a chain of the form identifier
generic-parameter-list? ( `.' identifier generic-parameter-list?
)*, returning t if the initial identifier was found and nil otherwise."
  (when (language-skip-identifier)
    (language-skip-generic-parameter-list)
    (when (language-skip-re "\\.")
      (language-skip-simple-type-name))
    t))

(defun language-skip-type-name ()
    "Hop over any comments, whitespace, and the name of a type if
one is present, returning t if so and nil otherwise"
  (language-skip-comments-and-space)
  (let ((found nil))
    ;; repeatedly
    (while
        (and
         ;; match a tuple or an identifier + optional generic param list
         (cond
          ((looking-at "[[(]")
           (forward-sexp)
           (setq found t))

          ((language-skip-simple-type-name)
           (setq found t)))

          ;; followed by "->"
         (prog2 (language-skip-re "\\?+")
             (language-skip-re "throws\\|rethrows\\|->")
           (language-skip-re "->") ;; accounts for the throws/rethrows cases on the previous line
           (language-skip-comments-and-space))))
    found))

(defun language-skip-constraint ()
    "Hop over a single type constraint if one is present,
returning t if so and nil otherwise"
  (language-skip-comments-and-space)
  (and (language-skip-type-name)
       (language-skip-re ":\\|==")
       (language-skip-type-name)))

(defun language-skip-where-clause ()
    "Hop over a where clause if one is present, returning t if so
and nil otherwise"
  (when (language-skip-re "\\<where\\>")
    (while (and (language-skip-constraint) (language-skip-re ",")))
    t))

(defun language-in-string-or-comment ()
  "Return non-nil if point is in a string or comment."
  (or (nth 3 (syntax-ppss)) (nth 4 (syntax-ppss))))

(defconst language-body-keyword-re
  "\\_<\\(var\\|fn\\|init\\|deinit\\|subscript\\)\\_>")

(defun language-hide-bodies ()
  "Hide the bodies of methods, functions, and computed properties.

Use `M-x hs-show-all' to show them again."
    (interactive)
  (hs-minor-mode)
  (save-excursion
    (save-match-data
      (goto-char (point-min))
      (while (search-forward-regexp language-body-keyword-re (point-max) :noerror)
        (when
            (and
             (not (language-in-string-or-comment))
             (let ((keyword (match-string 0)))
               ;; parse up to the opening brace
               (cond
                ((equal keyword "deinit") t)

                ((equal keyword "var")
                 (and (language-skip-identifier)
                      (language-skip-re ":")
                      (language-skip-type-name)))

                ;; otherwise, there's a parameter list
                (t
                 (and
                  ;; parse the function's base name or operator symbol
                  (if (equal keyword "fn") (forward-symbol 1) t)
                  ;; advance to the beginning of the function
                  ;; parameter list
                  (progn
                    (language-skip-generic-parameter-list)
                    (language-skip-comments-and-space)
                    (equal (char-after) ?\())
                  ;; parse the parameter list and any return type
                  (prog1
                    (language-skip-type-name)
                    (language-skip-where-clause))))))
             (language-skip-re "{"))
          (hs-hide-block :reposition-at-end))))))

(defun language-indent-line ()
  (interactive)
  (let (indent-level target-column)
    (save-excursion
      (widen)
      (setq indent-level (car (syntax-ppss (point-at-bol))))

      ;; Look at the first non-whitespace to see if it's a close paren
      (beginning-of-line)
      (skip-syntax-forward " ")
      (setq target-column
            (if (or (equal (char-after) ?\#) (looking-at "//:")) 0
              (* language-basic-offset
                 (- indent-level
                    (cond ((= (char-syntax (or (char-after) ?\X)) ?\))
                           1)
                          ((save-match-data
                             (looking-at
                              "case \\|default *:\\|[a-zA-Z_][a-zA-Z0-9_]*\\(\\s-\\|\n\\)*:\\(\\s-\\|\n\\)*\\(for\\|do\\|\\while\\|switch\\|repeat\\)\\>"))
                           1)
                          (t 0))))))
      (indent-line-to (max target-column 0)))
    (when (< (current-column) target-column)
      (move-to-column target-column)))
  )

;; Compilation error parsing
(push 'language0 compilation-error-regexp-alist)
(push 'language1 compilation-error-regexp-alist)
(push 'language-fatal compilation-error-regexp-alist)

(push `(language0
        ,(concat
     "^"
       "[ \t]+" "\\(?:(@\\)?"
       "[A-Z⚠️][A-Za-z0-9_]*@"
     ;; Filename \1
       "\\("
          "[0-9]*[^0-9\n]"
          "\\(?:"
             "[^\n :]" "\\|" " [^/\n]" "\\|" ":[^ \n]"
          "\\)*?"
       "\\)"
       ":"
       ;; Line number (\2)
       "\\([0-9]+\\)"
       ":"
       ;; Column \3
       "\\([0-9]+\\)"
       )
     1 2 3 0)
      compilation-error-regexp-alist-alist)

(push `(language1
        ,(concat
     "^"
       "[0-9]+[.][ \t]+While .* at \\[?"
     ;; Filename \1
       "\\("
          "[0-9]*[^0-9\n]"
          "\\(?:"
             "[^\n :]" "\\|" " [^/\n]" "\\|" ":[^ \n]"
          "\\)*?"
       "\\)"
       ":"
       ;; Line number (\2)
       "\\([0-9]+\\)"
       ":"
       ;; Column \3
       "\\([0-9]+\\)"
       )
     1 2 3 2)
      compilation-error-regexp-alist-alist)

(push `(language-fatal
        ,(concat
     "^\\(?:assertion failed\\|fatal error\\): \\(?:.*: \\)?file "
     ;; Filename \1
       "\\("
          "[0-9]*[^0-9\n]"
          "\\(?:"
             "[^\n :]" "\\|" " [^/\n]" "\\|" ":[^ \n]"
          "\\)*?"
       "\\)"
       ", line "
       ;; Line number (\2)
       "\\([0-9]+\\)"
       )
     1 2 nil 2)
      compilation-error-regexp-alist-alist)

;; Flymake support

(require 'flymake)

;; This name doesn't end in "function" to avoid being unconditionally marked as risky.
(defvar-local language-find-executable-fn 'executable-find
  "Function to find a command executable.
The function is called with one argument, the name of the executable to find.
Might be useful if you want to use a languagec that you built instead
of the one in your PATH.")
(put 'language-find-executable-fn 'safe-local-variable 'functionp)

(defvar-local language-syntax-check-fn 'language-syntax-check-directory
"Function to create the language command-line that syntax-checks the current buffer.
The function is called with two arguments, the languagec executable, and
the name of a temporary file that will contain the contents of the
current buffer.
Set to 'language-syntax-check-single-file to ignore other files in the current directory.")
(put 'language-syntax-check-fn 'safe-local-variable 'functionp)

(defvar-local language-syntax-check-args '("-typecheck")
  "List of arguments to be passed to languagec for syntax checking.
Elements of this list that are strings are inserted literally
into the command line.  Elements that are S-expressions are
evaluated.  The resulting list is cached in a file-local
variable, `language-syntax-check-evaluated-args', so if you change
this variable you should set that one to nil.")
(put 'language-syntax-check-args 'safe-local-variable 'listp)

(defvar-local language-syntax-check-evaluated-args
  "File-local cache of language arguments used for syntax checking
variable, `language-syntax-check-args', so if you change
that variable you should set this one to nil.")

(defun language-syntax-check-single-file (languagec temp-file)
  "Return a flymake command-line list for syntax-checking the current buffer in isolation"
  `(,languagec ("-typecheck" ,temp-file)))

(defun language-syntax-check-directory (languagec temp-file)
  "Return a flymake command-line list for syntax-checking the
current buffer along with the other language files in the same
directory."
  (let* ((sources nil))
    (dolist (x (directory-files (file-name-directory (buffer-file-name))))
      (when (and (string-equal "language" (file-name-extension x))
                 (not (file-equal-p x (buffer-file-name))))
        (setq sources (cons x sources))))
    `(,languagec ("-typecheck" ,temp-file ,@sources))))

(defun flymake-language-init ()
  (let* ((temp-file
          (flymake-init-create-temp-buffer-copy
           (lambda (x y)
             (make-temp-file
              (concat (file-name-nondirectory x) "-" y)
              (not :DIR_FLAG)
              ;; grab *all* the extensions; handles .code.gyb files, for example
              ;; whereas using file-name-extension would only get ".gyb"
              (replace-regexp-in-string "^\\(?:.*/\\)?[^.]*" "" (buffer-file-name)))))))
    (funcall language-syntax-check-fn
             (funcall language-find-executable-fn "languagec")
             temp-file)))

(require 'flymake-proc)
(add-to-list 'flymake-allowed-file-name-masks '(".+\\.code$" flymake-language-init))

(setq flymake-err-line-patterns
      (append
       (flymake-reformat-err-line-patterns-from-compile-el
        (mapcar (lambda (x) (assoc x compilation-error-regexp-alist-alist))
                '(language0 language1 language-fatal)))
       flymake-err-line-patterns))

(defgroup language nil
  "Major mode for editing language source files."
  :prefix "language-")

(provide 'language-mode)

;; end of language-mode.el
