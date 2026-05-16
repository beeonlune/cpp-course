#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

#ifdef TSP_BUILD_TESTS
#include <gtest/gtest.h>
#endif

namespace tsp
{
    constexpr std::size_t default_vertex_count = 10U;
    constexpr int min_edge_weight = 1;
    constexpr int max_edge_weight = 10;
    constexpr std::size_t default_start_vertex = 0U;

    using Graph = boost::adjacency_matrix<
        boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, int>>;

    using WeightMatrix = std::vector<std::vector<int>>;
    using Path = std::vector<std::size_t>;

    struct GraphData
    {
        Graph graph;
        WeightMatrix matrix;
    };

    struct TspResult
    {
        Path path;
        int cost{0};
    };

    [[nodiscard]] WeightMatrix make_empty_matrix(const std::size_t vertex_count)
    {
        return WeightMatrix(vertex_count, std::vector<int>(vertex_count, 0));
    }

    [[nodiscard]] GraphData build_complete_graph_from_matrix(const WeightMatrix& matrix)
    {
        const std::size_t vertex_count = matrix.size();

        if (vertex_count == 0U)
        {
            throw std::invalid_argument("matrix must not be empty");
        }

        for (const auto& row : matrix)
        {
            if (row.size() != vertex_count)
            {
                throw std::invalid_argument("matrix must be square");
            }
        }

        for (std::size_t i = 0U; i < vertex_count; ++i)
        {
            if (matrix[i][i] != 0)
            {
                throw std::invalid_argument("matrix diagonal must contain zeros");
            }

            for (std::size_t j = i + 1U; j < vertex_count; ++j)
            {
                if (matrix[i][j] != matrix[j][i])
                {
                    throw std::invalid_argument("matrix must be symmetric");
                }

                if (matrix[i][j] < min_edge_weight || matrix[i][j] > max_edge_weight)
                {
                    throw std::invalid_argument("edge weight is out of allowed range");
                }
            }
        }

        Graph graph(vertex_count);
        auto weight_map = boost::get(boost::edge_weight, graph);

        for (std::size_t i = 0U; i < vertex_count; ++i)
        {
            for (std::size_t j = i + 1U; j < vertex_count; ++j)
            {
                const auto [edge, inserted] = boost::add_edge(i, j, graph);
                if (!inserted)
                {
                    throw std::runtime_error("failed to add edge");
                }

                boost::put(weight_map, edge, matrix[i][j]);
            }
        }

        return GraphData{graph, matrix};
    }

    [[nodiscard]] GraphData generate_random_complete_graph(
        const std::size_t vertex_count,
        std::default_random_engine& engine)
    {
        if (vertex_count == 0U)
        {
            throw std::invalid_argument("vertex_count must be positive");
        }

        WeightMatrix matrix = make_empty_matrix(vertex_count);
        std::uniform_int_distribution<int> distribution(min_edge_weight, max_edge_weight);

        for (std::size_t i = 0U; i < vertex_count; ++i)
        {
            for (std::size_t j = i + 1U; j < vertex_count; ++j)
            {
                const int weight = distribution(engine);
                matrix[i][j] = weight;
                matrix[j][i] = weight;
            }
        }

        return build_complete_graph_from_matrix(matrix);
    }

    [[nodiscard]] int edge_weight(
        const Graph& graph,
        const std::size_t from,
        const std::size_t to)
    {
        const auto [edge, exists] = boost::edge(from, to, graph);

        if (!exists)
        {
            throw std::runtime_error("edge does not exist");
        }

        return boost::get(boost::edge_weight, graph, edge);
    }

    [[nodiscard]] int calculate_cycle_cost(const Graph& graph, const Path& path)
    {
        if (path.size() < 2U)
        {
            throw std::invalid_argument("path is too short");
        }

        int total_cost = 0;

        for (std::size_t i = 0U; i + 1U < path.size(); ++i)
        {
            total_cost += edge_weight(graph, path[i], path[i + 1U]);
        }

        return total_cost;
    }

    [[nodiscard]] bool is_hamiltonian_cycle(
        const Path& path,
        const std::size_t vertex_count,
        const std::size_t start_vertex)
    {
        if (vertex_count == 0U)
        {
            return false;
        }

        if (path.size() != vertex_count + 1U)
        {
            return false;
        }

        if (path.front() != start_vertex || path.back() != start_vertex)
        {
            return false;
        }

        std::vector<bool> seen(vertex_count, false);

        for (std::size_t i = 0U; i < vertex_count; ++i)
        {
            const std::size_t vertex = path[i];

            if (vertex >= vertex_count)
            {
                return false;
            }

            if (seen[vertex])
            {
                return false;
            }

            seen[vertex] = true;
        }

        return std::all_of(seen.begin(), seen.end(), [](const bool value)
        {
            return value;
        });
    }
    [[nodiscard]] TspResult solve_tsp_bruteforce(
        const Graph& graph,
        const std::size_t start_vertex = default_start_vertex)
    {
        const std::size_t vertex_count = boost::num_vertices(graph);

        if (vertex_count == 0U)
        {
            throw std::invalid_argument("graph must not be empty");
        }

        if (start_vertex >= vertex_count)
        {
            throw std::invalid_argument("start_vertex is out of range");
        }

        Path permutation;
        permutation.reserve(vertex_count - 1U);

        for (std::size_t vertex = 0U; vertex < vertex_count; ++vertex)
        {
            if (vertex != start_vertex)
            {
                permutation.push_back(vertex);
            }
        }

        TspResult best_result;
        best_result.cost = std::numeric_limits<int>::max();

        do
        {
            Path current_path;
            current_path.reserve(vertex_count + 1U);

            current_path.push_back(start_vertex);
            current_path.insert(current_path.end(), permutation.begin(), permutation.end());
            current_path.push_back(start_vertex);

            const int current_cost = calculate_cycle_cost(graph, current_path);

            if (current_cost < best_result.cost)
            {
                best_result.cost = current_cost;
                best_result.path = current_path;
            }
        }
        while (std::next_permutation(permutation.begin(), permutation.end()));

        return best_result;
    }

    [[nodiscard]] std::string matrix_to_string(const WeightMatrix& matrix)
    {
        std::ostringstream out;

        for (std::size_t i = 0U; i < matrix.size(); ++i)
        {
            for (std::size_t j = 0U; j < matrix[i].size(); ++j)
            {
                out << std::setw(2) << matrix[i][j];

                if (j + 1U != matrix[i].size())
                {
                    out << ' ';
                }
            }

            if (i + 1U != matrix.size())
            {
                out << '\n';
            }
        }

        return out.str();
    }

    [[nodiscard]] std::string path_to_string(const Path& path)
    {
        std::ostringstream out;

        for (std::size_t i = 0U; i < path.size(); ++i)
        {
            out << path[i];

            if (i + 1U != path.size())
            {
                out << " -> ";
            }
        }

        return out.str();
    }

    void print_demo_result(const GraphData& graph_data, const TspResult& result)
    {
        std::cout << "Adjacency matrix:\n";
        std::cout << matrix_to_string(graph_data.matrix) << "\n\n";
        std::cout << "Optimal path:\n";
        std::cout << path_to_string(result.path) << "\n\n";
        std::cout << "Total cost:\n";
        std::cout << result.cost << '\n';
    }

    [[nodiscard]] WeightMatrix make_test_matrix_ring_best(const std::size_t vertex_count)
    {
        WeightMatrix matrix = make_empty_matrix(vertex_count);

        for (std::size_t i = 0U; i < vertex_count; ++i)
        {
            for (std::size_t j = i + 1U; j < vertex_count; ++j)
            {
                matrix[i][j] = max_edge_weight;
                matrix[j][i] = max_edge_weight;
            }
        }

        for (std::size_t i = 0U; i < vertex_count; ++i)
        {
            const std::size_t next = (i + 1U) % vertex_count;
            matrix[i][next] = min_edge_weight;
            matrix[next][i] = min_edge_weight;
        }

        return matrix;
    }

    [[nodiscard]] WeightMatrix make_test_matrix_small()
    {
        return WeightMatrix{
            {0, 2, 9, 10},
            {2, 0, 6, 4},
            {9, 6, 0, 8},
            {10, 4, 8, 0}
        };
    }
}

#ifndef TSP_BUILD_TESTS

int main()
{
    try
    {
        std::random_device random_device;
        std::default_random_engine engine(random_device());

        const tsp::GraphData graph_data =
            tsp::generate_random_complete_graph(tsp::default_vertex_count, engine);

        const tsp::TspResult result =
            tsp::solve_tsp_bruteforce(graph_data.graph, tsp::default_start_vertex);

        tsp::print_demo_result(graph_data, result);
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}

#else

TEST(GraphConstructionTests, MatrixBuildsCorrectGraph)
{
    const tsp::WeightMatrix matrix = tsp::make_test_matrix_small();
    const tsp::GraphData graph_data = tsp::build_complete_graph_from_matrix(matrix);

    EXPECT_EQ(boost::num_vertices(graph_data.graph), 4U);
    EXPECT_EQ(tsp::edge_weight(graph_data.graph, 0U, 1U), 2);
    EXPECT_EQ(tsp::edge_weight(graph_data.graph, 1U, 3U), 4);
    EXPECT_EQ(tsp::edge_weight(graph_data.graph, 2U, 3U), 8);
}

TEST(GraphConstructionTests, RandomGraphHasSymmetricMatrixAndValidWeights)
{
    std::default_random_engine engine(12345U);
    const tsp::GraphData graph_data =
        tsp::generate_random_complete_graph(tsp::default_vertex_count, engine);

    ASSERT_EQ(graph_data.matrix.size(), tsp::default_vertex_count);

    for (std::size_t i = 0U; i < graph_data.matrix.size(); ++i)
    {
        ASSERT_EQ(graph_data.matrix[i].size(), tsp::default_vertex_count);
        EXPECT_EQ(graph_data.matrix[i][i], 0);

        for (std::size_t j = 0U; j < graph_data.matrix.size(); ++j)
        {
            EXPECT_EQ(graph_data.matrix[i][j], graph_data.matrix[j][i]);

            if (i != j)
            {
                EXPECT_GE(graph_data.matrix[i][j], tsp::min_edge_weight);
                EXPECT_LE(graph_data.matrix[i][j], tsp::max_edge_weight);
            }
        }
    }
}

TEST(CostCalculationTests, CalculatesCycleCostCorrectly)
{
    const tsp::WeightMatrix matrix = tsp::make_test_matrix_small();
    const tsp::GraphData graph_data = tsp::build_complete_graph_from_matrix(matrix);

    const tsp::Path path{0U, 1U, 3U, 2U, 0U};
    const int cost = tsp::calculate_cycle_cost(graph_data.graph, path);

    EXPECT_EQ(cost, 23);
}

TEST(ValidationTests, DetectsValidHamiltonianCycle)
{
    const tsp::Path path{0U, 1U, 2U, 3U, 0U};
    EXPECT_TRUE(tsp::is_hamiltonian_cycle(path, 4U, 0U));
}

TEST(ValidationTests, DetectsInvalidHamiltonianCycleWithRepeatedVertex)
{
    const tsp::Path path{0U, 1U, 1U, 3U, 0U};
    EXPECT_FALSE(tsp::is_hamiltonian_cycle(path, 4U, 0U));
}

TEST(ValidationTests, DetectsInvalidHamiltonianCycleWithWrongEnding)
{
    const tsp::Path path{0U, 1U, 2U, 3U, 2U};
    EXPECT_FALSE(tsp::is_hamiltonian_cycle(path, 4U, 0U));
}

TEST(TspSolverTests, FindsOptimalTourForSmallKnownGraph)
{
    const tsp::WeightMatrix matrix = tsp::make_test_matrix_small();
    const tsp::GraphData graph_data = tsp::build_complete_graph_from_matrix(matrix);

    const tsp::TspResult result = tsp::solve_tsp_bruteforce(graph_data.graph, 0U);

    EXPECT_EQ(result.cost, 23);
    EXPECT_TRUE(tsp::is_hamiltonian_cycle(result.path, 4U, 0U));
}

TEST(TspSolverTests, FindsOptimalTourForTenVertexGraphWithCheapRing)
{
    const tsp::WeightMatrix matrix =
        tsp::make_test_matrix_ring_best(tsp::default_vertex_count);
    const tsp::GraphData graph_data = tsp::build_complete_graph_from_matrix(matrix);

    const tsp::TspResult result = tsp::solve_tsp_bruteforce(graph_data.graph, 0U);

    EXPECT_EQ(result.cost, 10);
    EXPECT_TRUE(
        tsp::is_hamiltonian_cycle(
            result.path,
            tsp::default_vertex_count,
            0U));
}

TEST(FormattingTests, MatrixToStringProducesExpectedText)
{
    const tsp::WeightMatrix matrix{
        {0, 1},
        {1, 0}
    };

    const std::string text = tsp::matrix_to_string(matrix);
    EXPECT_EQ(text, " 0  1\n 1  0");
}

TEST(FormattingTests, PathToStringProducesExpectedText)
{
    const tsp::Path path{0U, 2U, 1U, 0U};
    const std::string text = tsp::path_to_string(path);

    EXPECT_EQ(text, "0 -> 2 -> 1 -> 0");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif