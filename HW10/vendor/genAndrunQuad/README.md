# Generating quad files and test run Quad programs

* Go in to the directory for your specific machine architecture.
* Use genQuad to generate quad programs (both text and xml) from fmj source file. Usage: `./genQuad filename`, without the `.fmj` suffix.
* Use `bash run_quad_text.sh full_text_quad_filename` to run the quad program (in text format that's printed by using quad.hh printing functions).
  