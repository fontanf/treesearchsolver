add_library(TreeSearchSolver_treesearchsolver)
target_sources(TreeSearchSolver_treesearchsolver PRIVATE
    common.cpp
    algorithm_formatter.cpp)
target_include_directories(TreeSearchSolver_treesearchsolver PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(TreeSearchSolver_treesearchsolver PUBLIC
    OptimizationTools::containers
    OptimizationTools::utils)
add_library(TreeSearchSolver::treesearchsolver ALIAS TreeSearchSolver_treesearchsolver)
set_target_properties(TreeSearchSolver_treesearchsolver PROPERTIES OUTPUT_NAME "treesearchsolver")
install(TARGETS TreeSearchSolver_treesearchsolver)
