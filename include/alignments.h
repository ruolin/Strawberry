/*
 * alignment.h
 *
 *  Created on: Nov 5, 2014
 *      Author: Ruolin Liu
 */

#ifndef STRAWB_ALIGNMENTS_H_
#define STRAWB_ALIGNMENTS_H_

#include <list>
#include <map>
#include <climits>
#include <atomic>
#include "read.hpp"
#include "contig.h"
#include "gff.h"
#include "isoform.h"

class Sample;

class FaInterface;

class FaSeqGetter;
using IntronMap = std::map<std::pair<uint, uint>, IntronTable>;

struct Segment {
   uint left;
   uint right;
   uint left_read_idx;
   uint right_read_idx;
   Strand_t strand;
   Segment(): left(std::numeric_limits<uint>::max()), right(0), left_read_idx(0), right_read_idx(0), strand(Strand_t::StrandUnknown) {}
   Segment(uint l, uint r, uint lidx, uint ridx, Strand_t s): left(l), right(r), left_read_idx(lidx),right_read_idx(ridx), strand(s){}
};

class HitCluster {
    friend Sample;
private:
    uint _leftmost;
    uint _rightmost;
    //int _plus_strand_num_hits;
    //int _minus_strand_num_hits;
    //Strand_t _first_encounter_strand;
    int _id = -1;
    std::string _gene_id;
    RefID _ref_id = -1;
    bool _final; // HitCluster is finished
    double _raw_mass = 0.0;
    std::unordered_map<ReadID, std::list<PairedHit>> _open_mates;
    std::vector<PairedHit> _hits;
    std::vector<PairedHit> _uniq_hits;
    std::vector<int> _read_ref_span;
    std::vector<Contig> _ref_mRNAs; // the actually objects are owned by Sample
    std::vector<GenomicFeature> _introns;
    std::vector<float> _dep_of_cov;

    void reweight_read(const std::unordered_map<std::string, double> &kmer_bias, int num_kmers);

    void reweight_read(bool weight_bais);
    //map<std::pair<int,int>,int> _current_intron_counter;
public:
    std::vector<Segment> _segs;
    std::map<Strand_t, std::map<GenomicFeature, int>> _strand_intron;
    decltype(auto) uniq_hits() const { return (_uniq_hits);}
    decltype(auto) id() const { return (_id);}
    double _weighted_mass = 0.0;
    //static const int _kMaxGeneLen = 1000000;
    //static const int _kMaxFragPerCluster = 100000;
    static const int _kMinFold4BothStrand = 10;

    HitCluster();

    RefID ref_id() const;

    std::pair<double,double> read_ref_span_mean_sd() const {
        return getMeanAndSd(_read_ref_span);
    };
    void ref_id(RefID id);

    uint left() const;

    void left(uint left);

    void right(uint right);

    uint right() const;

    int num_uniq_hits() const;
    int size() const {
        return _hits.size();
    }

    int len() const;

    std::string cluster_id() {
        if (_gene_id.empty()) return std::to_string(_id);
        else return _gene_id;
    }

    decltype(auto) gene_id() {
        return (_gene_id);
    }

    Strand_t ref_strand() const;

    Strand_t guessStrand() const;

    bool addHit(const PairedHit &hit);

    void count_current_intron(const std::vector<std::pair<uint, uint>> &cur_intron);

    void setBoundaries();

    void clearOpenMates();

    void refine_cluster();

    bool addOpenHit(const ReadHitPtr hit, bool extend_by_hit, bool extend_by_partner);

    int collapseAndFilterHits();

    bool overlaps(const HitCluster &rhs) const;

    bool hasRefmRNAs() const {
        return _ref_mRNAs.size() > 0;
    }

    std::vector<Contig> ref_mRNAs() const {
        return _ref_mRNAs;
    }

    void addRefContig(const Contig& contig);

    int numOpenMates() const {
        return _open_mates.size();
    }

    void addRawMass(double m) {
        _raw_mass += m;
    }

    void subRawMass(double m) {
        _raw_mass -= m;
    }

    double raw_mass() const {
        return _raw_mass;
    }

    double weighted_mass() const;

    void addWeightedMass(double m);


    //void count_current_intron(const ReadHit & hit);
    //bool current_intron_is_reliable() const;
    bool see_both_strands();

    void reset(uint old_left,
               uint old_right,
               RefID old_ref_id,
               int old_plus_hit,
               int old_minus_hit,
               Strand_t old_strand,
               double old_mass
    );

    void reset(uint old_left,
               uint old_right,
               RefID old_ref_id
    );


};

class LocusContext;
class Sample {
    std::atomic<int> _num_cluster;
    //uint _prev_pos = 0;
    //RefID _prev_hit_ref_id = -1; //used to judge if sam/bam is sorted.
    //uint _prev_hit_pos = 0; //used to judge if sam/bam is sorted.
    size_t _refmRNA_offset;
    bool _has_load_all_refs;
    std::string _current_chrom;
    std::atomic_int _total_mapped_reads = {0};


public:
    std::shared_ptr<HitFactory> _hit_factory;
    std::shared_ptr<InsertSize> _insert_size_dist = nullptr;
    std::shared_ptr<FaSeqGetter> _fasta_getter = nullptr;
    std::shared_ptr<FaInterface> _fasta_interface = nullptr;

    std::unordered_map<std::string, double> _kmer_bias;

    std::vector<Contig> _ref_mRNAs; // sort by seq_id in reference_table
    std::vector<Contig> _assembly;

    Sample(std::shared_ptr<HitFactory> hit_fac) :
            _refmRNA_offset(0),
            _has_load_all_refs(false),
            _hit_factory(move(hit_fac)) {
    }

    //int max_inner_dist() const;
    int total_mapped_reads() const;

    std::string sample_path() const {
        return _hit_factory->sample_path();
    }

    std::string sample_name() const {
        std::vector<std::string> fields;
        split(fileName(sample_path()), ".", fields);
        return fields.front();
    }

    bool load_chrom_fasta(RefID seq_id);

    std::string get_iso_seq(const std::shared_ptr<FaSeqGetter> &fa_getter, const Contig iso) const;

    bool hasLoadRefmRNAs() const {
        return _ref_mRNAs.size() > 0;
    }

    bool loadRefFasta(RefSeqTable &rt, const char *seqFile = NULL);

    bool loadRefmRNAs(std::vector<std::unique_ptr<GffTree>> &gseqs, RefSeqTable &rt);

    int addRef2Cluster(HitCluster &clusterOut);

    void reset_refmRNAs();

    double next_valid_alignment(ReadHit &readin);

    void rewindHit();
    void rewindHits(uint64_t pos);

    int nextCluster_denovo(HitCluster &clusterOut,
                           uint next_ref_start_pos = UINT_MAX,
                           RefID next_ref_start_ref = INT_MAX);

    int nextClusterRefDemand(HitCluster &clusterOut);

    int nextCluster_refGuide(HitCluster &clusterOut);

    void rewindReference(HitCluster &clusterOut, int num_regress);

    void mergeClusters(HitCluster &dest, HitCluster &resource);

    //void compute_doc_4_cluster(const HitCluster & hit_cluster, std::vector<float> &exon_doc,
    //map<std::pair<uint,uint>,IntronTable>& intron_counter, uint &small_overhang);
    std::vector<Isoform> quantifyCluster(const RefSeqTable &ref_t, const std::shared_ptr<HitCluster> cluster,
                         const std::vector<Contig> &assembled_transcripts, FILE* plogfile, FILE* fragfile) const;


    void procSample(FILE *f, FILE *log, FILE* fragfile);

    void assembleSample(FILE *log);
    void inspect_read_len();

    std::vector<Contig> runFlowAlgorithm(const Strand_t& strand, const std::vector<Contig>& hits,
                                      const std::map<std::pair<uint,uint>, IntronTable> &intron_counter,
                                      const std::vector<GenomicFeature> &exons);

    std::vector<Contig> assembleCluster(const RefSeqTable &ref_t, std::shared_ptr<HitCluster> cluster, FILE *plogfile);

    void finalizeCluster(std::shared_ptr<HitCluster>, bool);

    std::vector<Contig> assembleContig(const uint l, const uint r, const Strand_t, const std::vector<Contig>&);
    void addAssembly(const std::vector<Contig>&);
    void fragLenDist(const RefSeqTable &ref_t, const std::vector<Contig> &isoforms,
                     const std::shared_ptr<HitCluster> cluster, FILE *plogfile);
    void preProcess(FILE *log);
    void printContext(const LocusContext& est, const std::shared_ptr<HitCluster> cluster,
                      const std::shared_ptr<FaSeqGetter> & fa_getter, FILE *fragfile) const;
};


bool hit_lt_cluster(const ReadHit &hit, const HitCluster &cluster, uint olap_radius);

bool hit_gt_cluster(const ReadHit &hit, const HitCluster &cluster, uint olap_radius);

bool hit_complete_within_cluster(const PairedHit &hit, const HitCluster &cluster, uint olap_radius);

double compute_doc(const uint left,
                   const uint right,
                   const std::vector<Contig> &hits,
                   std::vector<float> &exon_doc,
                   IntronMap &intron_doc,
                   uint smallOverhang);

void filter_intron(const std::string& current_chrom, const uint cluster_left,
                   const uint read_abs_len, const std::vector<float> &exon_doc, IntronMap& intron_counter);

#endif
