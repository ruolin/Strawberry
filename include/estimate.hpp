/*
 * qp.h
 *
 *  Created on: Oct 15, 2015
 *      Author: ruolin
 */

#ifndef QP_H_
#define QP_H_
#include "contig.h"
#include "read.hpp"
#include "bias.hpp"


class LocusContext {
//typedef CGAL::Quadratic_program<double> Program;
//typedef CGAL::Quadratic_program_solution<ET> Solution;
   static const double _kMinTPM;
   static constexpr double _kMinFPKM = 1e-2;

   std::shared_ptr<InsertSize> _insert_size_dist;
   int _read_len;
   FILE* _p_log_file;
   std::vector<ExonBin> exon_bins;
   std::map<int, std::set<int>> iso_2_bins_map;

   void set_maps(
           const int& iso_id,
           const int& fg_len,
           const float& mass,
           const Contig& read,
           const std::set<std::pair<uint, uint>>& coords){

      if(coords.empty()) return;

      ExonBin eb(coords);
      eb.add_frag(read);
      eb.add_frag_len(iso_id, fg_len, mass);

      int ebid = UniqPushAndReturnIdx<ExonBin>(eb, exon_bins);

      auto find_it2 = iso_2_bins_map.find(iso_id);
      if (find_it2 == iso_2_bins_map.end()){
         std::set<int> bin_ids = {ebid};
         iso_2_bins_map.emplace(iso_id, bin_ids);
      }
      else{
         find_it2->second.insert(ebid);
      }
   }

   void assign_exon_bin(
           const std::vector<Contig> &hits,
           const std::vector<Isoform> &transcripts,
           const std::vector<GenomicFeature> & exon_segs);

public:
   LocusContext(std::shared_ptr<InsertSize> insert_size,
                              int read_len, FILE* tracker,
                              const std::vector<Contig> &hits,
                              const std::vector<Isoform> &transcripts,
                              const std::vector<GenomicFeature> &exon_segs):
         _insert_size_dist(insert_size), _read_len(read_len), _p_log_file(tracker)
   {
      assign_exon_bin(hits, transcripts, exon_segs);
   }

   void test();
   void overlap_exons(const std::vector<GenomicFeature>& exons,
                     const Contig& read,
                     std::set<std::pair<uint,uint>> &coords);

   void set_theory_bin_weight(const std::map<int, int> &iso_2_len_map,
                          const std::vector<Isoform>& isoforms);

   void set_empirical_bin_weight(const std::map<int, int> &iso_2_len_map, const int m);

   void calculate_raw_iso_counts();

   bool estimate_abundances(const double mass,
                            std::map<int, int>& iso_2_len_map,
                            std::vector<Isoform>& isoforms,
                            bool with_bias_correction,
                            const std::shared_ptr<FaSeqGetter> & fa_getter);


   std::vector<std::vector<double>> calculate_bin_bias(const std::shared_ptr<FaSeqGetter> &fa_getter) const {

      std::vector<std::vector<double>> bias;
      for (auto it = exon_bins.cbegin(); it != exon_bins.cend(); ++it) {
//#ifdef DEBUG
//      for(auto e: it->first){
//         cout<<"exon bin "<<e.first<<"="<<e.second<<endl;
//      }
//#endif
         std::vector<double> b;
         b.reserve(3);
         double gc = it->bin_gc_content(fa_getter);
         b.push_back(gc);
         b.push_back(gc * gc);
         b.push_back(gc * gc * gc);
         b.push_back(it->bin_len());
         b.push_back(it->avg_frag_len());
//      for(auto c:it->first){
//         cout<<"exon bin: " <<c.first<<"-"<<c.second<<endl;
//      }
         bias.push_back(b);
      }
      return bias;
   }

};





class EmSolver{
   const double TOLERANCE = std::numeric_limits<double>::denorm_min();
   std::vector<double> _theta_after_zero;
   std::vector<int> _u; // observed data vector
   std::vector<std::vector<double>> _F; // sampling rate matrix, \alpha
   std::vector<std::vector<double>> _B; // bias matrix, \beta
   std::vector<std::vector<double>> _U; // hidden unobserved data matrix.
   static constexpr int _max_iter_num = 1000;
   static constexpr int _max_bias_it_num = 10;
   static constexpr int _max_theta_it_num = 5000;
   static constexpr int _max_out_it_num = 100;
   static constexpr double _theta_change_limit = 1e-2;
   static constexpr double _bias_change_limit = 1e-2;
public:
   std::vector<double> _theta;
   std::vector<double> _bias;
   EmSolver() = default;
   bool init( const int num_iso,
         const std::vector<int> &count,
         const std::vector<std::vector<double>> &model);

   bool init( const int num_iso,
         const std::vector<int> &count,
         const std::vector<std::vector<double>> &model,
         const std::vector<std::vector<double>> &bias);

   bool run();
};

int gap_ef(const int l_left, const int l_right, const int l_int, const int rl, const int gap);
int no_gap_ef(const int l_left, const int l_right, const int l_int, const int fl);
uint generate_pair_end(const Contig& ct, const Contig& s, int span,  SingleOrit_t orit, Contig & mp);

#endif /* QP_H_ */
