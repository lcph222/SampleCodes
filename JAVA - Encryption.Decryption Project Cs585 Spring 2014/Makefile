CLASS_DIR=build/class/
JARFILE=build/target.jar
SRC_DIR=src/
MAIN_CLASS=KeyExchange
.PHONY: src
jar: $(JARFILE)
$(JARFILE): src
    jar cvfe $(JARFILE) $(MAIN_CLASS) -C $(CLASS_DIR) .
src:
    @$(MAKE) -C $(SRC_DIR)

clean:
    rm -rf build
