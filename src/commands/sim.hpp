/* phyLS: powerful heightened yielded Logic Synthesis
 * Copyright (C) 2022 */

/**
 * @file sim.hpp
 *
 * @brief logic network simulation
 *
 * @author Homyoung
 * @since  2022/12/14
 */

#ifndef SIM_HPP
#define SIM_HPP

#include <mockturtle/algorithms/simulation.hpp>

using namespace std;

namespace alice {

class sim_command : public command {
 public:
  explicit sim_command(const environment::ptr &env)
      : command(env, "logic network simulation") {
    add_flag("-a, --aig_network", " simulate current aig network");
    add_flag("-x, --xmg_network", " simulate current xmg network");
    add_flag("-p, --partial_simulate",
             " simulate current logic network using partial simulator ");
  }

 protected:
  void execute() {
    clock_t begin, end;
    double totalTime;
    if (is_set("xmg_network")) {
      begin = clock();
      if (is_set("partial_simulate")) {
        xmg_network xmg = store<xmg_network>().current();
        std::vector<kitty::partial_truth_table> pats(xmg.num_pis());
        for (auto i = 0; i < xmg.num_pis(); i++)
          pats[i].add_bits(0x12345678, 32);
        partial_simulator sim(pats);
        unordered_node_map<kitty::partial_truth_table, xmg_network>
            node_to_value(xmg);
        simulate_nodes(xmg, node_to_value, sim);

        xmg.foreach_po([&](auto const &f) {
          std::cout << "tt: 0x"
                    << (xmg.is_complemented(f) ? ~node_to_value[f]
                                               : node_to_value[f])
                           ._bits[0]
                    << std::endl;
        });
      } else {
        xmg_network xmg = store<xmg_network>().current();
        default_simulator<kitty::dynamic_truth_table> sim(xmg.num_pis());
        unordered_node_map<kitty::dynamic_truth_table, xmg_network>
            node_to_value(xmg);
        simulate_nodes(xmg, node_to_value, sim);

        xmg.foreach_gate([&](auto const &n) {
          std::cout << "node " << n << " tt: " << node_to_value[n]._bits[0]
                    << std::endl;
        });

        xmg.foreach_po([&](auto const &f) {
          std::cout << "PO tt: "
                    << (xmg.is_complemented(f) ? ~node_to_value[f]
                                               : node_to_value[f])
                           ._bits[0]
                    << std::endl;
        });
        // const auto tt = simulate<kitty::dynamic_truth_table>( xmg, sim )[0];
        // std::cout << "tt: 0x" << tt._bits[0] << std::endl;
      }
      end = clock();
      totalTime = (double)(end - begin) / CLOCKS_PER_SEC;
    } else if (is_set("aig_network")) {
      begin = clock();
      aig_network aig = store<aig_network>().current();
      default_simulator<kitty::dynamic_truth_table> sim(aig.num_pis());
      const auto tt = simulate<kitty::dynamic_truth_table>(aig, sim)[0];
      std::cout << "tt: " << tt._bits[0] << std::endl;
      end = clock();
      totalTime = (double)(end - begin) / CLOCKS_PER_SEC;
    } else {
      std::cout << "At least one store should be specified, see 'sim -h' for "
                   "help. \n";
    }
    cout.setf(ios::fixed);
    cout << "[CPU time]   " << setprecision(2) << totalTime << " s" << endl;
  }
};

ALICE_ADD_COMMAND(sim, "Verification")
}  // namespace alice

#endif
