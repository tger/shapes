;;; shapes-mode.el --- Emacs major mode for editing Shapes programs

;; Copyright (C) 2008 Tobias Gerdin

;; This file is part of Shapes.

;; Shapes is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; any later version.

;; Shapes is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with Shapes.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;; This mode is very much work in progress.  It does not handle automatic
;; indentation of Shapes programs nor does it offer any motion commands adapted
;; to such.  That said, it does support compilation-mode, flymake-mode,
;; comment-dwim, Imenu, skeleton-pairs, outline-mode (but see documentation for
;; `shapes-outline-regexp') and viewing output through doc-view.
;;
;; TODO
;; - Motion commands
;; - Bind mode context-menu to C-mouse 3, featuring send-to-region etc.
;; - Handle automatic indentation of operators such as <<, &
;; - Hide compilation buffer if a doc-view buffer is visible and there are no
;;errors when recompiling.
;; - PDF sync between source and output (path control points etc).
;; - Automatically pretty-print #, -> (use abbrevs?)
;; - Convert Shapes ref manual to info and hook up to Info-Look
;; - Skeletons?
;; - Eldoc mode?
;; - Some sort of camelCase support a la subword mode of cc-mode
;;
;; LIMITATIONS
;; - Flymake does not make use of column number information
;; - Nested strings are not font-locked correctly (disabled)

;; BUGS
;; - Investigate mismatch between Shapes and Emacs column numbers.
;;   Note: Emacs column 0 means that *point* is before the first char.  Column 1
;; * point is after first char (and the cursor is ON char 2).
;; - Handle the case when shapes-mode is started in a buffer that isn't saved.
;; - Handle the case when shapes-mode is activated on a buffer that doesn't end
;; with .sh(ape|ext), so that its encoding is not set to utf-8 correctly, that
;; is change the buffer encoding to utf-8 if it is set to something else.
;; - Code brackets are indented one column to much, since they are treated as
;; function calls.
;; - The use of forward-sexp for indentation means that some Shapes expressions
;; are not moved over correctly, such as '[...].foldl'.

;;; Installation:

;; To install, put this file somewhere in your load path and just require
;; 'shapes-mode from your ~/.emacs or equivalent, like so:
;;
;; (require 'shapes-mode)
;;
;; - Ghostscript (with "gs" somewhere in your PATH) is required for display of inline
;; drawable values in interactive mode.
;; - doc-view.el is required for viewing Shapes output graphically in a separate buffer.

;; Code tested only on GNU Emacs 22 and 23.

;;; Code:

(defcustom shapes-compiler-command "shapes"
  "Name of the Shapes compiler executable, and any options to pass to it."
  :type 'string)

;; (defcustom shapes-unicode-pretty-print t
;;   "Whether to pretty-print Shapes idenifiers using their Unicode
;; equivalents or not."
;;   :type 'boolean)

(defvar shapes-basic-indent-width 2)

(defvar shapes-file-regex "\\.sh\\(ape\\|ext\\)$"
  "Regular expression matching Shapes source files.")

(defvar shapes-compiler-message-parse-element
	'("\\(.*\\):\\([0-9]+\\)(\\([0-9]+\\)-\\([0-9]+\\)):" 1 2 (3 . 4))
	"List containing regular expression and match indices for parsing Shapes
	compiler messages.  Used by compilation- and flymake modes.")

(defvar shapes-identifier-re
	"[a-zA-Z0-9_?]+"
	"Regular expression matching Shapes identifiers.")

(defvar shapes-outline-regexp
	(concat shapes-identifier-re ":")
	"Regular expression matching an `outline-mode' header.

We only attempt to match a top-level heading here, since outline-mode's
\"ingenious\" use of the length of the match to calculate nesting does not work
well with Shapes' binding syntax.

This implies that it only makes sense to use work with the outline in terms of
entries, since the nesting of headings will be random.")

;; Associate Shapes source files with shapes-mode.
(push (cons shapes-file-regex 'shapes-mode) auto-mode-alist)
;; The Shapes compiler wants its input utf-8, LF line-endings (for now).
(modify-coding-system-alist 'file shapes-file-regex 'utf-8-unix)

(defvar shapes-mode-syntax-table
  (let ((st (make-syntax-table)))
		;;; Strings
		;; We make use of the generic string delimiter syntax class since the normal
		;; string-quote syntax class requires opening and closing delimiters to be
		;; the same.
		;; Note: It seems impossible to handle nested string delimiters (apparently
		;; it is on the Emacs todo list).
		;; 
		;; An alternative to this method would be to use search-based font-lock, but
		;; I believe that it is not possible to write recursive regular expressions
		;; in Emacs, so we could not handle nested delimiters this way either.

    ;; Normal strings (does not handle nesting!)
    (modify-syntax-entry ?` "|" st)			; String start delimiter
		(modify-syntax-entry ?´ "|" st)			; String close delimiter
    ;; Poor man's strings are handled using search-based font lock

    ;; So that it doesn't interfere with syntax properties-based highlighting
    ;; for strings:
    (modify-syntax-entry ?\" "." st)

		;;; Operators
		;; Uses punctuation character syntax class.
		;; (modify-syntax-entry ?| "." st)

    ;; Remove the syntax flags from these characters as we handle comments using
    ;; search-based fontification and syntax properties instead.
    (modify-syntax-entry ?/ "." st)
    (modify-syntax-entry ?* "." st)

		;; add more here ...
		st)
  "Syntax table used while in `shapes-mode'.")

(defconst shapes-mode-font-lock-keywords
  '(
    ;; Single-line comments.
    ("|\\*\\*.*" . font-lock-comment-face)

    ;; Operators should be painted as keywords, I guess.
    ;; "\\\\" "->"

    ;; I guess it make some sense to use function-name-face for bindings
    ;; and variable-name-face for states.
    ;; (eval
    ;; 			(lambda ()
    ;; 				(list (concat "\\(#" shapes-identifier-re "\\):")
    ;; 							1 'font-lock-variable-name-face)))
    ;; 		 (eval
    ;; 			(lambda ()
    ;; 				;; It should probably not match dynamic var bindings?
    ;; 				(list (concat "\\(" shapes-identifier-re "\\):")
    ;; 							1 'font-lock-function-name-face)))
    ;; 		 (eval
    ;; 			(lambda ()
    ;; 				;; newText, TeX et al.
    ;; 				(list (concat "(\\(" shapes-identifier-re "\\)")
    ;; 							1 'font-lock-keyword-face)))
		 
    ;; Preprocessor directives
    ("##needs" . font-lock-preprocessor-face)

;;; 		 ("~" . font-lock-negation-char-face) ; doesn't work?
	)
  "Shape keywords and their corresponding font-lock face.")

(defconst shapes-mode-syntactic-keywords
  '(
    ;; Multi-line comments
    ("\\(/\\)\\*\\*.*\\*\\*\\(/\\)" (1 "<") (2 ">"))
    
    ;; Poor man's strings: ("string")
    ("\\((\\)\"" 1 "|")
    ("\"\\()\\)" 1 "|")

    ;; Data strings: "{string}
    ;; Note: Nesting is not properly handled
    ("\\(\"\\){.*\\(}\\)" (1 "|") (2 "|"))
    )
  "`font-lock-syntactic-keywords` for Shapes.")

(defvar shapes-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\C-c\C-c" 'shapes-compile)
		(define-key map "\C-c\C-r" 'shapes-send-region)
		;; The below is disabled because it relies on shapes-end-of-defun, which is
		;; not finished.
		;;(define-key map "\M-\C-x" 'shapes-send-definition)
		(when (featurep 'doc-view)		; Emacs 22 and lower does not ship with
					; doc-view
      (define-key map "\C-c\C-v" 'shapes-view))
    (mapc (lambda (elt)
	    (define-key map (string elt) 'skeleton-pair-insert-maybe))
	  "`([{")
    ;; (define-key map "\C-cl" 'shapes-lambda)
    map))

(defun shapes-compile ()
  "Compiles the source file."
  (interactive)
  (compile (concat shapes-compiler-command " " (buffer-file-name))))

(defun shapes-view ()
  "Compiles the source file and, if compilation is successful,
views it using doc-view."
  (interactive)
  ;; Here we basically pass the code to view the output as a continuation to the
  ;; compile function since compilation is performed asynchronously in Emacs.
  (add-to-list
   'compilation-finish-functions
   (lambda (comp-buf exit-msg)
     ;; I am confused as to which buffer is really active when we are
     ;; applied. (buffer-name) reports *compilation* but (window-buffer) reports
     ;; the Shapes program buffer.
     (setq compilation-finish-functions (cdr compilation-finish-functions))
     (when (equal exit-msg "finished\n")
       (let ((output (concat (file-name-sans-extension
                              (buffer-file-name (window-buffer)))
                             ".pdf")))
	 (with-selected-window
	     (next-window)		; This way image will hopefully reuse
					; the compilation buffer.
	   (doc-view t output))))))						; I Emacs 23 one should make use of
																				; find-file instead.
  (shapes-compile))

(defun shapes-preoutput-filter (output)
	"Parses output from Shapes looking for '#File' references, and
replaces them with the image contents.

Images are converted to PNG format using Ghostscript and
displayed using the IMAGE text display property."
	(if (string-match "\\(#File: \\(.*\\)\\)\n" output)
			(let* ((shapes-output-file (match-string 2 output))
						 (shapes-mode-out-file-name
							(concat (file-name-sans-extension shapes-output-file)
											".png")))
				(condition-case nil
						(progn
							;; Inspired by doc-view
							(call-process "gs"
														nil nil nil
														"-sDEVICE=png16m"	; PNG format, of some sort.
														(concat "-sOutputFile="
																		shapes-mode-out-file-name)
														shapes-output-file)
							(let ((disp-spec (create-image
																shapes-mode-out-file-name
																'png
																nil
																;; To prevent Emacs from fetching a cached copy. This way
																;; the image spec will not be EQUAL to the former one.
																:dummy-prop (random))))
								(put-text-property (match-beginning 1) (match-end 1)
																	 'display disp-spec output)))
					;; GS was probaly not found, fail silently.
					(error))))
	output)

(defun run-shapes ()
	"Starts up Shapes.

Also displays the Shapes window, and if run interactively,
selects it."
  (interactive)
	(setq shapes-buffer
				(make-comint "shapes" shapes-compiler-command nil
										 "--interactive"
										 "--i-format-prompt=shapes> "))
	(with-current-buffer shapes-buffer
		(set (make-local-variable 'prev-image-file-name) nil)
		(add-hook 'comint-preoutput-filter-functions
							'shapes-preoutput-filter
							nil t))
	(funcall 
	 (if (interactive-p)
			 'pop-to-buffer
		 'display-buffer)
	 shapes-buffer))

(defmacro with-shapes-running-and-visible (&rest body)
	`(progn
		 ;; We could possibly rely on the fact that comint-mode seems to check if
		 ;; what you want to run is already running.
		 (if (and (boundp 'shapes-buffer)
							(buffer-live-p shapes-buffer))
				 (display-buffer shapes-buffer)
			 (run-shapes)) 
		 ,@body))

;; Apparently Shapes expect each input on a single line. Would be nice if it
;; waited until it had something it was certain didn't make any sense (ie try to
;; parse input until it is syntactially correct). Otherwise, there needs to be a
;; continuation marker before EOL, or some explicit way of terminating the
;; input.
(defun shapes-send-region (start end)
	"Send the current region to Shapes.
Also displays the Shapes window, but does not select it."
  (interactive "r")
	(run-shapes)
	(comint-send-region shapes-buffer start end)
	(unless (char-equal (char-before end) ?\n)
		(comint-send-string shapes-buffer "\n")))

(defun shapes-send-definition ()
	"Sends the definition under point to Shapes."
	(interactive)
	(apply (function shapes-send-region)
				 (save-excursion
					 ;; In case we were already at the beginning of the defun,
					 ;; and would have skipped back to the former one.
					 (forward-char) 										
					 (beginning-of-defun)						 
					 (let ((start (point)))
						 (end-of-defun)
						 (list start (point))))))

(defun shapes-beginning-of-defun (&optional arg)
  "Move backward to the beginning of a defun.
A line beginning with something apart from whitespace or a
closing brace is considered to be the beginning of a defun.

The above means that for now comments are not skipped."
  (interactive "p")
  ;; Or shoulde we search for top-level bindings bound to functions
	;; only?  I.e. match identifier: \ arg1 .. argn -> body-expr
	;;(re-search-backward (concat "^" shapes-identifier-re) nil t (or
	;;arg 1)))
	(re-search-backward "^[^[:blank:]\n}]" nil t (or arg 1)))

(defun shapes-end-of-defun (&optional arg)
	"Move forward to the end of a defun.
A defun is considered to extend to next line beginning with a
something apart from whitespace, minus any blank lines in between.

The above means that for now comments are not skipped.

Note: BUGGY, do not use."
	(interactive "p")
	;; Right now we are at the beginning of the defun.
	;; If we are on an opening brace, move to the matching one
	(if (looking-at "{")
			(forward-sexp)
		(progn 
			(forward-line)						 ; A defun must span at least one line
			;; Find the next line beginning with non-whitespace
			(re-search-forward "^[^[:blank:]\n]" nil t (or arg 1))
			(beginning-of-line)
			(re-search-backward "^.+")				; Skip emtpy lines
			(end-of-line))))	

;; (defun shapes-indent-line ()
;;   "Indents a line of Shapes code.

;; Idea: Treat [, {, and ( as sexpressions and use pp-sexp!

;; This code could really use some clean-up."
;; 	(defun point-to-column (p)
;; 		(save-excursion
;; 			(goto-char p)
;; 			(current-column)))
;; 	(defun calc-function-indent ()
;; 		;; Extract dynamically scoped parameters?
;; 		(goto-char open-pos)
;; 		(narrow-to-region (point) (line-end-position))
;; 		(forward-char)
;; 		(skip-chars-forward " \t")
;; 		;;   [           foo   bar
;; 		;;   ^: open     ^: s1 ^: s2
;; 		(+ open-col													; We need to add this because
;; 																				; narrow-to-region will cause
;; 																				; current-column not to see it.
;; 			 (let ((s1-col (current-column)))
;; 				 (forward-sexp)									; I probably need to supply my own
;; 																				; forward-sexp function, as this one
;; 																				; fails on [...].foldl for instance.
;; 				 (if (= (current-column) s1-col) ; Did we not move over something?
;; 						 shapes-basic-indent-width
;; 					 (progn
;; 						 (skip-chars-forward " \t")
;; 						 (let ((s2-col (current-column)))
;; 							 (forward-sexp)
;; 							 (if (= (current-column) s2-col)
;; 									 ;; No more expression.
;; 									 (+ s1-col shapes-basic-indent-width)
;; 								 ;; There was another expression, align with it.
;; 								 s2-col)))))))
;;   (let ((pos (- (point-max) (point)))		; We store pos relative to end of file,
;; 																				; so that we can go back to it even
;; 																				; after indentation has inserted (before
;; 																				; point).
;; 				(beg (line-beginning-position)))
;;     ;; We really want to indent w.r.t to the first thing on the line.
;;     (goto-char beg)
;; 		;; Wipe previous identation.
;; 		(delete-horizontal-space)
;;   	;; Note, the below 'looking-back'-based method is ugly! (and probably slow).
;; 		;; Should probably try to find a better one, possibly involving some real
;; 		;; parsing.
;; 		(if (looking-back "\\\\.*?\\(->\\|→\\)\n")		; Function definition?
;; 				(indent-to (+ (point-to-column (match-beginning 0))
;; 											shapes-basic-indent-width))
;; 			;; No, use normal braces-based indent.																	 
;; 			(let* ((state (syntax-ppss (point)))
;; 						 (depth (car state))
;; 						 (open-pos (elt state 1)))
;; 				(unless (zerop depth)
;; 					;; Highlight opening brace, for debugging.
;; 					(when (boundp 'sm-debug)			; Ie shapes-mode-debug.
;; 						(princ state)
;; 						(let ((open-ov (make-overlay open-pos (1+ open-pos))))
;; 							(overlay-put open-ov 'face 'highlight)
;; 							(sit-for 1)
;; 							(delete-overlay open-ov)))
;; 					(let ((open-col (point-to-column open-pos)))
;; 						(indent-to
;; 						 (save-excursion
;; 							 (save-restriction
;; 								 (if (looking-at "}\\|<)")
;; 										 ;; Closing braces for code brackets and structures.
;; 										 ;; Align with opening brace.
;; 										 open-col
;; 									 (calc-function-indent)))))))
;; 				;; Restore previous cursor position.
;; 				(goto-char (- (point-max) pos))))))

; set parse-sexp-lookup-properties 
(defun shapes-indent-line ()
  "Indents current line according to Shapes indentation standards."

  (defun prev-line-indent ()
    "Returns indent of previous line. Moves point."
    (beginning-of-line 0)
    (back-to-indentation)
    (current-column))

  (defun point-to-column (p)
		(save-excursion
			(goto-char p)
			(current-column)))

  ;; We store pos relative to end of file so that we can go back to it even
  ;; after indentation has been inserted (before point)
  (let ((pos (- (point-max) (point))))
    ;; We want to indent w.r.t to the first thing on the line.
    (goto-char (line-beginning-position))
    ;; Wipe previous identation.
    (delete-horizontal-space)
    (indent-to
     (let* ((state (syntax-ppss (point)))
            (depth (car state))
            (open-pos (elt state 1)))
       (save-excursion
         (save-restriction
           (cond
            ;; closing brace, align with opening brace
            ;; TODO: handle plethora of strings literals such as "{ str }, `string'
            ((looking-at "\\(\\s)\\|<)\\)")
             (point-to-column open-pos))
            ;; continuing operator, add basic indent (could add more operators here)
            ((and (looking-back "\\(→\\|.>\\|<<\\)\n" (- (point) 3))
                  ;; this one may not be wanted? or maybe only if opening brace is in column 1?
                  (not (looking-at "{")))
             (+ (prev-line-indent) shapes-basic-indent-width))
            ;; align with << on previous line (if present), otherwise add basic-indent to last
            ;; line's indent
            ((looking-at "<<")
             (if (re-search-backward "<<" (line-beginning-position 0) t)
                 (point-to-column (match-beginning 0))
               (+ (prev-line-indent) shapes-basic-indent-width)))
            ;; Default case, indent according to brace level
            (t
             (if (zerop depth)
                 0
               (+ (point-to-column open-pos) shapes-basic-indent-width))
             ))))))
    ;; Restore old pos
    (goto-char (- (point-max) pos))))

;; (defun shapes-indent-line ()
;;   "Indents current line according to Shape indentation standards."
;;   (interactive)
;;   ;; We store pos relative to end of file so that we can go back to it even
;;   ;; after indentation has been inserted (before point)
;;   (let ((pos (- (point-max) (point)))
;;         (beg (line-beginning-position))
;;         (new-indent (shapes-calc-line-indent)))
;;     (goto-char beg)
;;     (indent-to new-indent)
;;     ;; Restore old pos
;;     (goto-char pos)))

;; I ripped this one off from the Perl one in flymake.el.
(defun shapes-flymake-init ()
	(let* ((temp-file   (flymake-init-create-temp-buffer-copy
                       'flymake-create-temp-inplace))
				 (local-file  (file-relative-name
                       temp-file
                       (file-name-directory buffer-file-name))))
		(list shapes-compiler-command (list local-file))))

(defun shapes-mode ()
  "Major mode for editing Shapes programs.
\\{shapes-mode-map}"
  (interactive)
  (kill-all-local-variables)
  (setq major-mode 'shapes-mode)
  (setq mode-name "Shapes")
  (use-local-map shapes-mode-map)
  (set-syntax-table shapes-mode-syntax-table)
	(setq font-lock-defaults
        (list shapes-mode-font-lock-keywords
              nil nil nil nil
              '(font-lock-syntactic-keywords . shapes-mode-syntactic-keywords)))
  (set (make-local-variable 'parse-sexp-lookup-properties) t)

  ;; Skeletons
  (set (make-local-variable 'skeleton-pair-alist)
       '((?` _ ?´)											; for strings
				 (?{ \n > _ \n ?} >))						; for code brackets
			 )

  ;; (define-skeleton shapes-lambda "Function template skeleton."
  ;;     "Formal parameters: "
  ;;     "\\ " str (if shapes-unicode-pretty-print " → " " -> ") _)

  ;; (when shapes-unicode-pretty-print
	;;     (setq abbrev-mode t))

	(set (make-local-variable 'beginning-of-defun-function)
			 'shapes-beginning-of-defun)
	(set (make-local-variable 'end-of-defun-function)
			 'shapes-end-of-defun)
  
  (setq comment-start-skip "/\\*\\*+ *\\||\\*\\*+ *")
  (setq comment-start "|**")

	(setq indent-tabs-mode nil)
  (setq indent-line-function (function shapes-indent-line))

  ;; Compilation-mode support
  ;; Example data:
  ;;  /Users/tger/stroke.shape:1(8-10): The unit b is unbound
  (eval-after-load 'compile
    '(progn
       (add-to-list 'compilation-error-regexp-alist
										shapes-compiler-message-parse-element)
       ;; The Shapes compiler works this way
       (setq compilation-error-screen-columns nil)))

	;; Flymake support
	(eval-after-load 'flymake
		'(progn
			 (add-to-list 'flymake-allowed-file-name-masks
										(list shapes-file-regex 'shapes-flymake-init))
			 (add-to-list 'flymake-err-line-patterns
										shapes-compiler-message-parse-element)))

  ;; Recognizes top-level Shapes identifiers.
  (setq imenu-generic-expression `((nil ,(concat "^\\(" shapes-identifier-re "\\):") 1)
																	 (nil ,(concat "^dynamic\\s-+\\(@"
																								 shapes-identifier-re
																								 "\\)") 1)))
	(setq outline-regexp shapes-outline-regexp)

  (unless (or (file-exists-p "makefile")
	      (file-exists-p "Makefile"))
    (set (make-local-variable 'compile-command)
	 (concat shapes-compiler-command " "
		 (file-name-sans-extension buffer-file-name)))))

(provide 'shapes-mode)

;; Local Variables: **
;; coding:utf-8! **
;; tab-width:2 **
;; End: **