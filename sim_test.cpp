// exec: ./tuo-test , verbose: ./tuo-test --log_level=all
// set-iterations ./tuo-test 100  , default = 10 // more than 100 cause errors
#ifdef TEST
#ifdef NQUEST // only without quests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE sim

#include <chrono>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <iostream>
#include <fstream>
#include <iostream>
#include <exception>
#include <sstream>
#include "tyrant.h"
#include "tyrant_optimize.h"
#include "sim.h"
#include "read.h"

using namespace std;
namespace bdata = boost::unit_test::data;
typedef std::tuple<FinalResults<long double>, std::string, double, std::string> Result; // score, output, time, result_deck
int iter = 1000;
unsigned seed = 0;
// limit for float diffing
// disable
double eps = 1;
// enable
// double eps = 0.0000001;

// pipe output: https://stackoverflow.com/questions/5405016/can-i-check-my-programs-output-with-boost-test
struct ios_redirect
{
    ios_redirect(std::streambuf *new_buffer, std::ostream &ios)
        : old(ios.rdbuf(new_buffer)), iios(ios)
    {
    }

    ~ios_redirect()
    {
        iios.rdbuf(old);
    }

private:
    std::streambuf *old;
    std::ostream &iios;
};

struct TestInfo
{
    std::string your_deck, enemy_deck, bge;
};
std::ostream &operator<<(std::ostream &os, const TestInfo &ti)
{
    return os << "Your Deck: " << ti.your_deck << "; Enemy Deck: " << ti.enemy_deck << "; BGE: " << ti.bge;
}

inline Result run_sim(int argc, const char **argv, bool pipe_output = true)
{
    Result res;
    std::string rdeck = "";
    FinalResults<long double> fr;
    debug_str.clear();
    //

    auto start_time = std::chrono::system_clock::now();
    // pipe output
    std::stringstream output;
    std::stringstream eoutput;
    {
        ios_redirect guard2(eoutput.rdbuf(), std::cerr); // block warnings
        if (pipe_output)
        {
            ios_redirect guard1(output.rdbuf(), std::cout);

            //////////////////////
            // only single thread for string stream build
            //////////////////////
            const char **param = new const char *[argc + 2];
            for (int i = 0; i < argc; i++)
                param[i] = const_cast<char *>(argv[i]);
            param[argc] = const_cast<char *>("-t");
            param[argc + 1] = const_cast<char *>("1");
            auto rett = run(argc + 2, param);
            fr = rett.second;
            // result to string
            auto drc = rett;
            std::stringstream oss;
            if (drc.first->commander)
                oss << drc.first->commander->m_name << ", ";
            if (drc.first->alpha_dominion)
                oss << drc.first->alpha_dominion->m_name << ", ";
            print_cards_inline(drc.first->cards, oss, drc.first);
            rdeck = oss.str();
        }
        else
        {
            // no guard here
            //////////////////////
            // only single thread, else crashes
            //////////////////////
            const char **param = new const char *[argc + 2];
            for (int i = 0; i < argc; i++)
                param[i] = const_cast<char *>(argv[i]);
            param[argc] = const_cast<char *>("-t");
            param[argc + 1] = const_cast<char *>("1");
            auto rett = run(argc + 2, param);
            fr = rett.second;
            // result to string
            auto drc = rett;
            std::stringstream oss;
            if (drc.first->commander)
                oss << drc.first->commander->m_name << ", ";
            if (drc.first->alpha_dominion)
                oss << drc.first->alpha_dominion->m_name << ", ";
            print_cards_inline(drc.first->cards, oss, drc.first);
            rdeck = oss.str();
        }
    }

    auto end_time = std::chrono::system_clock::now();
    std::chrono::duration<double> delta_t = (end_time - start_time);
    res = std::make_tuple(fr, "\n" + debug_str + output.str(), delta_t.count(), rdeck);
    return res;
}

inline void check_win(Result result)
{
    BOOST_CHECK_MESSAGE(
        1 - eps <= std::get<0>(result).wins &&
            eps >= std::get<0>(result).losses &&
            eps >= std::get<0>(result).draws,
        std::get<1>(result));
    // BOOST_CHECK(100==result.points);
}
// inline void check_win_sim(const char* your_deck, const char* enemy_deck, const char* bge="") {
inline void check_win_sim(TestInfo ti)
{
    /////////////
    // Max. Iter == 100, else check_win fails with integer vs double equal in check_win
    ////////////
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    s = std::to_string(seed);
    char *iii = new char[s.length()];
    strcpy(iii, s.c_str());
    const char *argv[] = {"tuo", ti.your_deck.c_str(), ti.enemy_deck.c_str(), "-e", ti.bge.c_str(), "sim", ii, "seed", iii, "prefix", "tests/sim/"}; // much output on error?! // better 100 iterations for test, 10 for checking errors
    // const char* argv[] = {"tuo",ti.your_deck.c_str(),ti.enemy_deck.c_str(),"-e",ti.bge.c_str(),"sim", ii,"seed", iii}; //much output on error?! // better 100 iterations for test, 10 for checking errors
    Result result(run_sim(sizeof(argv) / sizeof(*argv), argv));
    delete[] ii;
    delete[] iii;
    // result.second += "\nTest: " + ti.your_deck + "; " + ti.enemy_deck + "; " + ti.bge;
    check_win(result);
}
inline double time_db_sim(std::string gnt1, std::string gnt2)
{
    /////////////
    // Max. Iter == 100, else check_win fails with integer vs double equal in check_win
    ////////////
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    s = std::to_string(seed);
    char *iii = new char[s.length()];
    strcpy(iii, s.c_str());
    const char *argv[] = {"tuo", gnt1.c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/db/", "db", "no-db-load"}; // much output on error?! // better 100 iterations for test, 10 for checking errors
    // const char* argv[] = {"tuo",ti.your_deck.c_str(),ti.enemy_deck.c_str(),"-e",ti.bge.c_str(),"sim", ii,"seed", iii}; //much output on error?! // better 100 iterations for test, 10 for checking errors
    Result result(run_sim(sizeof(argv) / sizeof(*argv), argv));
    delete[] ii;
    delete[] iii;
    // result.second += "\nTest: " + ti.your_deck + "; " + ti.enemy_deck + "; " + ti.bge;
    // check_win(result);
    return std::get<2>(result);
}
inline double time_db(std::string gnt1, std::string gnt2)
{
    /////////////
    // Max. Iter == 100, else check_win fails with integer vs double equal in check_win
    ////////////
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    s = std::to_string(seed);
    char *iii = new char[s.length()];
    strcpy(iii, s.c_str());
    const char *argv[] = {"tuo", gnt1.c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/db/", "db"}; // much output on error?! // better 100 iterations for test, 10 for checking errors
    // const char* argv[] = {"tuo",ti.your_deck.c_str(),ti.enemy_deck.c_str(),"-e",ti.bge.c_str(),"sim", ii,"seed", iii}; //much output on error?! // better 100 iterations for test, 10 for checking errors
    Result result(run_sim(sizeof(argv) / sizeof(*argv), argv));
    delete[] ii;
    delete[] iii;
    // result.second += "\nTest: " + ti.your_deck + "; " + ti.enemy_deck + "; " + ti.bge;
    // check_win(result);
    return std::get<2>(result);
}
inline double time_ml_sim(std::string gnt1, std::string gnt2)
{
    /////////////
    // Max. Iter == 100, else check_win fails with integer vs double equal in check_win
    ////////////
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    s = std::to_string(seed);
    char *iii = new char[s.length()];
    strcpy(iii, s.c_str());
    const char *argv[] = {"tuo", gnt1.c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/ml/", "no-db-load"}; // much output on error?! // better 100 iterations for test, 10 for checking errors
    // const char* argv[] = {"tuo",ti.your_deck.c_str(),ti.enemy_deck.c_str(),"-e",ti.bge.c_str(),"sim", ii,"seed", iii}; //much output on error?! // better 100 iterations for test, 10 for checking errors
    Result result(run_sim(sizeof(argv) / sizeof(*argv), argv));
    delete[] ii;
    delete[] iii;
    // result.second += "\nTest: " + ti.your_deck + "; " + ti.enemy_deck + "; " + ti.bge;
    // check_win(result);
    return std::get<2>(result);
}
inline double time_ml(std::string gnt1, std::string gnt2)
{
    /////////////
    // Max. Iter == 100, else check_win fails with integer vs double equal in check_win
    ////////////
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    s = std::to_string(seed);
    char *iii = new char[s.length()];
    strcpy(iii, s.c_str());
    const char *argv[] = {"tuo", gnt1.c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/ml/", "no-db", "ml"}; // much output on error?! // better 100 iterations for test, 10 for checking errors
    // const char* argv[] = {"tuo",ti.your_deck.c_str(),ti.enemy_deck.c_str(),"-e",ti.bge.c_str(),"sim", ii,"seed", iii}; //much output on error?! // better 100 iterations for test, 10 for checking errors
    Result result(run_sim(sizeof(argv) / sizeof(*argv), argv));
    delete[] ii;
    delete[] iii;
    // result.second += "\nTest: " + ti.your_deck + "; " + ti.enemy_deck + "; " + ti.bge;
    // check_win(result);
    return std::get<2>(result);
}

inline void genetic(std::string gnt1, std::string gnt2)
{
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    const char *argv[] = {"tuo", gnt1.c_str(), gnt2.c_str(), "brawl", "genetic", ii};
    Result result(run_sim(sizeof(argv) / sizeof(*argv), argv, false));
    std::ofstream mf;
    mf.open("out.csv", std::ios_base::app);
    mf << gnt1 << ";" << gnt2 << ";" << std::get<0>(result).points << ";" << std::get<2>(result) << std::endl;
    mf.close();
}

inline void check_algo(std::string gnt1, std::string gnt2, std::string algo)
{
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    s = std::to_string(seed);
    char *iii = new char[s.length()];
    strcpy(iii, s.c_str());
    const char *argv1[] = {"tuo", gnt1.c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/algo/", "no-db"};
    Result result_sim(run_sim(sizeof(argv1) / sizeof(*argv1), argv1));
    // Do the algo
    const char *argv2[] = {"tuo", gnt1.c_str(), gnt2.c_str(), algo.c_str(), ii, "seed", iii, "prefix", "tests/algo/", "no-db"};
    Result result(run_sim(sizeof(argv2) / sizeof(*argv2), argv2));
    // Rerun sim with optimized deck to check if algo produced wrong sim result
    const char *argv3[] = {"tuo", std::get<3>(result).c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/algo/", "no-db"};
    Result result_opt(run_sim(sizeof(argv3) / sizeof(*argv3), argv3));
    delete[] ii;
    delete[] iii;
    BOOST_CHECK_MESSAGE(std::get<0>(result_sim).wins <= std::get<0>(result_opt).wins,
                        std::get<1>(result_sim) + "\n" + std::get<1>(result_opt) + "\nWrongly from " + algo + ": " +
                            std::to_string(std::get<0>(result_sim).wins) + ">" + std::to_string(std::get<0>(result_opt).wins));
}
inline void check_anneal(std::string gnt1, std::string gnt2)
{
    std::string algo = "anneal";
    double anneal_temp = 100;
    double anneal_temp_down = 0.01;
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    s = std::to_string(seed);
    char *iii = new char[s.length()];
    strcpy(iii, s.c_str());
    s = std::to_string(anneal_temp);
    char *iv = new char[s.length()];
    strcpy(iv, s.c_str());
    s = std::to_string(anneal_temp_down);
    char *v = new char[s.length()];
    strcpy(v, s.c_str());
    const char *argv1[] = {"tuo", gnt1.c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/algo/", "no-db"};
    Result result_sim(run_sim(sizeof(argv1) / sizeof(*argv1), argv1));
    const char *argv2[] = {"tuo", gnt1.c_str(), gnt2.c_str(), algo.c_str(), ii, iv, v, "seed", iii, "prefix", "tests/algo/", "no-db"};
    Result result(run_sim(sizeof(argv2) / sizeof(*argv2), argv2));
    // Rerun sim with optimized deck to check if algo produced wrong sim result
    const char *argv3[] = {"tuo", std::get<3>(result).c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/algo/", "no-db"};
    Result result_opt(run_sim(sizeof(argv3) / sizeof(*argv3), argv3));
    delete[] ii;
    delete[] iii;
    delete[] iv;
    delete[] v;
    BOOST_CHECK_MESSAGE(std::get<0>(result_sim).wins <= std::get<0>(result_opt).wins,
                        std::get<1>(result_sim) + "\n" + std::get<1>(result_opt) + "\nWrongly from " + algo + ": " +
                            std::to_string(std::get<0>(result_sim).wins) + ">" + std::to_string(std::get<0>(result_opt).wins));
}
inline void check_climbex(std::string gnt1, std::string gnt2)
{
    std::string algo = "climbex";
    int init = 10;
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    s = std::to_string(seed);
    char *iii = new char[s.length()];
    strcpy(iii, s.c_str());
    s = std::to_string(init);
    char *iv = new char[s.length()];
    strcpy(iv, s.c_str());
    const char *argv1[] = {"tuo", gnt1.c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/algo/", "no-db"};
    Result result_sim(run_sim(sizeof(argv1) / sizeof(*argv1), argv1));
    const char *argv2[] = {"tuo", gnt1.c_str(), gnt2.c_str(), algo.c_str(), iv, ii, "seed", iii, "prefix", "tests/algo/", "no-db"};
    Result result(run_sim(sizeof(argv2) / sizeof(*argv2), argv2));
    // Rerun sim with optimized deck to check if algo produced wrong sim result
    const char *argv3[] = {"tuo", std::get<3>(result).c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "prefix", "tests/algo/", "no-db"};
    Result result_opt(run_sim(sizeof(argv3) / sizeof(*argv3), argv3));
    delete[] ii;
    delete[] iii;
    delete[] iv;
    BOOST_CHECK_MESSAGE(std::get<0>(result_sim).wins <= std::get<0>(result_opt).wins,
                        std::get<1>(result_sim) + "\n" + std::get<1>(result_opt) + "\nWrongly from " + algo + ": " +
                            std::to_string(std::get<0>(result_sim).wins) + ">" + std::to_string(std::get<0>(result_opt).wins));
}

inline void check_climb_forts(std::string gnt1, std::string gnt2, std::string yf, std::string ef)
{
    std::string algo = "climb_forts";
    int init = 10;
    string s = std::to_string(iter);
    char *ii = new char[s.length()];
    strcpy(ii, s.c_str());
    s = std::to_string(seed);
    char *iii = new char[s.length()];
    strcpy(iii, s.c_str());
    s = std::to_string(init);
    char *iv = new char[s.length()];
    strcpy(iv, s.c_str());
    const char *argv1[] = {"tuo", gnt1.c_str(), gnt2.c_str(), "sim", ii, "seed", iii, "yf", yf.c_str(), "yfpool", "2", "ef", ef.c_str(), "efpool", "2", "prefix", "tests/algo/", "no-db"};
    Result result_sim(run_sim(sizeof(argv1) / sizeof(*argv1), argv1));
    const char *argv2[] = {"tuo", gnt1.c_str(), gnt2.c_str(), algo.c_str(), iv, ii, "seed", iii, "yf", yf.c_str(), "yfpool", "2", "ef", ef.c_str(), "efpool", "2", "prefix", "tests/algo/", "no-db"};
    Result result_opt(run_sim(sizeof(argv2) / sizeof(*argv2), argv2));
    // Rerun sim with optimized deck to check if algo produced wrong sim result
    // const char* argv3[] = {"tuo",gnt1.c_str(),gnt2.c_str(),"sim",ii, "seed", iii,  "yf",std::get<3>(result).c_str(), "yfpool","2", "ef", ef.c_str(), "efpool", "2","prefix" , "tests/algo/", "no-db"};
    // Result result_opt(run_sim(sizeof(argv3)/sizeof(*argv3),argv3));
    delete[] ii;
    delete[] iii;
    delete[] iv;
    BOOST_CHECK_MESSAGE(std::get<0>(result_sim).wins <= std::get<0>(result_opt).wins,
                        std::get<1>(result_sim) + "\n" + std::get<1>(result_opt) + "\nWrongly from " + algo + ": " +
                            std::to_string(std::get<0>(result_sim).wins) + ">" + std::to_string(std::get<0>(result_opt).wins));
}

std::vector<TestInfo> read_test_file(const std::string filename)
{
    std::vector<TestInfo> ret;
    std::ifstream test_file(filename);
    if (test_file.good())
    {
        try
        {
            while (test_file && !test_file.eof())
            {
                TestInfo ti;
                std::string line, yd, ed, bg;
                std::getline(test_file, line);
                if (is_line_empty_or_commented(line))
                {
                    continue;
                }
                std::istringstream iss(line);
                std::getline(iss, ti.your_deck, ';');
                std::getline(iss, ti.enemy_deck, ';');
                std::getline(iss, ti.bge, ';');
                ret.push_back(ti);
            }
        }
        catch (std::exception &e)
        {
        }
    }
    return ret;
}

BOOST_AUTO_TEST_SUITE(test)
BOOST_AUTO_TEST_CASE(test_init)
{
    init();

    seed = std::chrono::system_clock::now().time_since_epoch().count() * 2654435761;
    if (boost::unit_test::framework::master_test_suite().argc >= 2)
    {
        iter = atoi(boost::unit_test::framework::master_test_suite().argv[1]);
    }
    if (boost::unit_test::framework::master_test_suite().argc >= 3)
    {
        seed = atoi(boost::unit_test::framework::master_test_suite().argv[2]);
    }
    //BOOST_TEST_MESSAGE("ITER: " << iter);
    BOOST_TEST_MESSAGE("SEED: " << seed);
    BOOST_CHECK(1 == 1); //..
}

BOOST_AUTO_TEST_SUITE(test_algo)
BOOST_AUTO_TEST_CASE(test_algo_init)
{
    iter = 1000;
}
BOOST_AUTO_TEST_SUITE(test_algo_climb)
BOOST_AUTO_TEST_CASE(test_algo_climb)
{
    check_algo("Mission#134", "Mission#135", "climb");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_algo_anneal)
BOOST_AUTO_TEST_CASE(test_algo_anneal)
{
    check_anneal("Mission#134", "Mission#135");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_algo_climbex)
BOOST_AUTO_TEST_CASE(test_algo_climbex)
{
    check_climbex("Mission#134", "Mission#135");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_algo_genetic)
BOOST_AUTO_TEST_CASE(test_algo_genetic)
{
    check_algo("Mission#134", "Mission#135", "genetic");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_algo_beam)
BOOST_AUTO_TEST_CASE(test_algo_beam)
{
    check_algo("Mission#134", "Mission#135", "beam");
}
BOOST_AUTO_TEST_SUITE_END()

// TODO Fix this test
// BOOST_AUTO_TEST_SUITE(test_algo_climb_forts)
// BOOST_AUTO_TEST_CASE(test_algo_climb_forts)
//{
//        check_climb_forts("Mission#135","Mission#135","LC#2, TC#2, IB#2, DF#2","LC#2, TC#2, IB#2, DF#2");
//}
// BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_sim)
BOOST_AUTO_TEST_CASE(test_sim_init)
{
    iter = 100;
    debug_print++;
    debug_cached++;
    debug_line = true;
}
/////////////////////////////////////
// Test Cases !should! be very close fights for maximum sensitivity of errors
/////////////////////////////////////
BOOST_AUTO_TEST_SUITE(test_single_units)
BOOST_DATA_TEST_CASE(test_single_units, bdata::make(read_test_file("tests/test_sinlge_units.csv")), ti)
{
    check_win_sim(ti);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_multi_units)
BOOST_DATA_TEST_CASE(test_multi_units, bdata::make(read_test_file("tests/test_multi_units.csv")), ti)
{
    check_win_sim(ti);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_bges)
BOOST_DATA_TEST_CASE(test_bges, bdata::make(read_test_file("tests/test_bges.csv")), ti)
{
    check_win_sim(ti);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_whole_decks)
BOOST_DATA_TEST_CASE(test_whole_decks, bdata::make(read_test_file("tests/test_whole_decks.csv")), ti)
{
    check_win_sim(ti);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_crashes)
BOOST_AUTO_TEST_CASE(test_crashes)
{
    eps = 1.; // only check for crashes now
    std::vector<std::vector<TestInfo>> aati;
    // aati.emplace_back(read_test_file("tests/test_whole_decks.csv"));
    // aati.emplace_back(read_test_file("tests/test_bges.csv"));
    // aati.emplace_back(read_test_file("tests/test_multi_units.csv"));
    aati.emplace_back(read_test_file("tests/test_crash.csv"));
    std::string decks = "";
    for (auto t = aati.begin(); t != aati.end(); ++t)
        for (auto tt = t->begin(); tt != t->end(); ++tt)
            decks += tt->your_deck + ";" + tt->enemy_deck + ";";
    TestInfo ti;
    ti.your_deck = decks;
    ti.enemy_deck = decks;
    ti.bge = "";
    check_win_sim(ti);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_db)
BOOST_AUTO_TEST_CASE(test_db_init)
{
    iter = 100000;
}
BOOST_AUTO_TEST_SUITE(test_db_scaling)
BOOST_AUTO_TEST_CASE(test_db_scaling)
{
    auto t1 = time_db_sim("Sir Alaric the Swift-1, Broodmother's Nexus-6, Mystic Gatekeeper-6, Ruthless Pursuer-6, Maculakornos-6, Primal Yeren-6, Megalift Foundry-6, Kinaxa Soulspark-6, Mangler of Existence-6, Mangler of Existence-6, Eranore's Obstructor-6, Eranore's Obstructor-6, ", "Sir Alaric the Swift-1, Broodmother's Nexus-6, Mystic Gatekeeper-6, Ruthless Pursuer-6, Maculakornos-6, Primal Yeren-6, Megalift Foundry-6, Kinaxa Soulspark-6, Mangler of Existence-6, Mangler of Existence-6, Eranore's Obstructor-6, Eranore's Obstructor-6,");
    auto t2 =     time_db("Sir Alaric the Swift-1, Broodmother's Nexus-6, Mystic Gatekeeper-6, Ruthless Pursuer-6, Maculakornos-6, Primal Yeren-6, Megalift Foundry-6, Kinaxa Soulspark-6, Mangler of Existence-6, Mangler of Existence-6, Eranore's Obstructor-6, Eranore's Obstructor-6, ", "Sir Alaric the Swift-1, Broodmother's Nexus-6, Mystic Gatekeeper-6, Ruthless Pursuer-6, Maculakornos-6, Primal Yeren-6, Megalift Foundry-6, Kinaxa Soulspark-6, Mangler of Existence-6, Mangler of Existence-6, Eranore's Obstructor-6, Eranore's Obstructor-6,");
    BOOST_CHECK_MESSAGE(t1 > t2, "DB time improvement failed: " + std::to_string(t1) + " > " + std::to_string(t2));
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(test_ml)
BOOST_AUTO_TEST_CASE(test_ml_init)
{
    iter = 100000;
}
BOOST_AUTO_TEST_SUITE(test_ml_scaling)
BOOST_AUTO_TEST_CASE(test_ml_scaling)
{
    auto t1 = time_ml_sim("Sir Alaric the Swift-1, Broodmother's Nexus-6, Mystic Gatekeeper-6, Ruthless Pursuer-6, Maculakornos-6, Primal Yeren-6, Megalift Foundry-6, Kinaxa Soulspark-6, Mangler of Existence-6, Mangler of Existence-6, Eranore's Obstructor-6, Eranore's Obstructor-6, ", "Sir Alaric the Swift-1, Broodmother's Nexus-6, Mystic Gatekeeper-6, Ruthless Pursuer-6, Maculakornos-6, Primal Yeren-6, Megalift Foundry-6, Kinaxa Soulspark-6, Mangler of Existence-6, Mangler of Existence-6, Eranore's Obstructor-6, Eranore's Obstructor-6,");
    auto t2 = time_ml("Sir Alaric the Swift-1, Broodmother's Nexus-6, Mystic Gatekeeper-6, Ruthless Pursuer-6, Maculakornos-6, Primal Yeren-6, Megalift Foundry-6, Kinaxa Soulspark-6, Mangler of Existence-6, Mangler of Existence-6, Eranore's Obstructor-6, Eranore's Obstructor-6, ", "Sir Alaric the Swift-1, Broodmother's Nexus-6, Mystic Gatekeeper-6, Ruthless Pursuer-6, Maculakornos-6, Primal Yeren-6, Megalift Foundry-6, Kinaxa Soulspark-6, Mangler of Existence-6, Mangler of Existence-6, Eranore's Obstructor-6, Eranore's Obstructor-6,");
    BOOST_CHECK_MESSAGE(t1 > t2, "ML time improvement failed: " + std::to_string(t1) + " > " + std::to_string(t2));
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
#endif
#endif
