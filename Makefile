CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Iinclude -MMD -MP
BUILD    := build

-include Makefile.local

SFML_LIBS  := -lsfml-graphics -lsfml-window -lsfml-system
GTEST_LIBS := -lgtest -lgtest_main -lpthread

CORE_SRC := $(wildcard src/*.cpp) $(wildcard src/movement/*.cpp)
CORE_OBJ := $(patsubst src/%.cpp,$(BUILD)/core/%.o,$(CORE_SRC))

GUI_SRC := $(wildcard gui/*.cpp) $(wildcard gui/input/*.cpp)
GUI_OBJ := $(patsubst gui/%.cpp,$(BUILD)/gui/%.o,$(GUI_SRC))

TEST_SRC := $(wildcard tests/*.cpp)
TEST_OBJ := $(patsubst tests/%.cpp,$(BUILD)/tests/%.o,$(TEST_SRC))

.PHONY: all clean test test-parte1a test-parte1b test-parte2 test-parte3 test-integracion

all: hive

hive: $(CORE_OBJ) $(GUI_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@.tmp $(LDFLAGS) $(SFML_LIBS)
	mv -f $@.tmp $@

test: hive_tests
	./hive_tests

test-parte1a: hive_tests
	./hive_tests --gtest_filter='Player.*'

test-parte1b: hive_tests
	./hive_tests --gtest_filter='QueenMovement.*'

test-parte2: hive_tests
	./hive_tests --gtest_filter='BeetleMovement.*:GrasshopperMovement.*:Piece.*:StrategyFactory.*-StrategyFactory.CreatesLadybugMovementForLadybug'

test-parte3: hive_tests
	./hive_tests --gtest_filter='BoardGraphAdapter*:BoardConnectivity.*:BoardCanMove.*:LadybugMovement.*:StrategyFactory.CreatesLadybugMovementForLadybug'

test-integracion: hive_tests
	./hive_tests --gtest_filter='Game.*:PillbugAbility.*:MosquitoAbility.*'

hive_tests: $(CORE_OBJ) $(BUILD)/gui/HexLayout.o $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@.tmp $(LDFLAGS) $(GTEST_LIBS)
	mv -f $@.tmp $@

$(BUILD)/core/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/gui/%.o: gui/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Igui -c $< -o $@

$(BUILD)/tests/%.o: tests/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Igui -c $< -o $@

clean:
	rm -rf build hive hive_tests

-include $(CORE_OBJ:.o=.d) $(GUI_OBJ:.o=.d) $(TEST_OBJ:.o=.d)
