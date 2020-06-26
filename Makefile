TARGET = simulator

all: ${TARGET}

# -Werror -Wall -Wextra -pedantic -Wcast-align -Wcast-qual 
# -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2
# -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnoexcept
# -Wold-style-cast -Woverloaded-virtual -Wredundant-decls
# -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5
# -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses 
# -fdiagnostics-show-option


${TARGET}: branch_prediction_simulator.cpp
	g++ -std=c++14 branch_prediction_simulator.cpp -Wall -Wextra -pedantic-errors -o ${TARGET}

clean:
	rm -f ${TARGET}
