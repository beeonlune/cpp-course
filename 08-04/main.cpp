#include <cassert>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>

const std::string target_sequence = "methinksitislikeaweasel";
const std::size_t sequence_length = 23;
const std::size_t population_size = 100;
const double mutation_probability = 0.05;

const char alphabet_start = 'a';
const int alphabet_size = 26;

char get_random_letter(std::default_random_engine & engine)
{
    const double min_bound = 0.0;
    const double max_bound = static_cast < double > (alphabet_size);
    std::uniform_real_distribution < double > distribution(min_bound, max_bound);

    int random_offset = static_cast < int > (distribution(engine));

    // safety check
    if (random_offset >= alphabet_size)
    {
        random_offset = alphabet_size - 1;
    }

    return static_cast < char > (alphabet_start + random_offset);
}

// new character is different from original
char get_different_random_letter(std::default_random_engine & engine, const char original_char)
{
    char new_char = original_char;

    while (new_char == original_char)
    {
        new_char = get_random_letter(engine);
    }

    return new_char;
}

std::string generate_initial_string(std::default_random_engine & engine)
{
    std::string result = "";
    for (std::size_t i = 0; i < sequence_length; ++i)
    {
        result += get_random_letter(engine);
    }
    return result;
}

std::size_t calculate_metric(const std::string & current, const std::string & target)
{
    std::size_t distance = 0;

    for (std::size_t i = 0; i < current.length(); ++i)
    {
        if (current[i] != target[i])
        {
            distance++;
        }
    }

    return distance;
}

void run_dawkins_weasel_tests()
{
    std::random_device random_device;
    std::default_random_engine engine(random_device());

    const double prob_min = 0.0;
    const double prob_max = 1.0;
    std::uniform_real_distribution <double> probability_distribution(prob_min, prob_max);

    std::string current_string = generate_initial_string(engine);

    bool target_reached = false;
    const std::size_t perfect_match_metric = 0;

    while (!target_reached)
    {
        std::cout << current_string << '\n';

        const std::size_t initial_metric = calculate_metric(current_string, target_sequence);

        if (initial_metric == perfect_match_metric)
        {
            target_reached = true;
            break;
        }

        std::string best_candidate = "";

        std::size_t lowest_metric = sequence_length + 1;

        for (std::size_t copy_index = 0; copy_index < population_size; ++copy_index)
        {
            std::string mutated_copy = current_string;

            for (std::size_t char_index = 0; char_index < sequence_length; ++char_index)
            {
                if (probability_distribution(engine) < mutation_probability)
                {
                    mutated_copy[char_index] = get_different_random_letter(engine, mutated_copy[char_index]);
                }
            }

            const std::size_t current_metric = calculate_metric(mutated_copy, target_sequence);

            if (current_metric < lowest_metric)
            {
                lowest_metric = current_metric;
                best_candidate = mutated_copy;
            }

            if (lowest_metric == perfect_match_metric)
            {
                break;
            }
        }
        current_string = best_candidate;
    }
}

int main()
{
    run_dawkins_weasel_tests();

    return 0;
}
