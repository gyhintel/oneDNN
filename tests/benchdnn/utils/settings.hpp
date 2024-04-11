/*******************************************************************************
* Copyright 2022-2024 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef UTILS_SETTINGS_HPP
#define UTILS_SETTINGS_HPP

struct base_settings_t {
    struct settings_attributes_t {
        void clear() { attrs_.clear(); }

        void init(const std::vector<attr_t::arg_scales_t> &scales,
                const std::vector<attr_t::zero_points_t> &zero_points,
                const std::vector<attr_t::post_ops_t> &post_ops,
                const std::vector<dnnl_scratchpad_mode_t> &scratchpad_mode,
                const std::vector<attr_t::fpmath_mode_t> &fpmath_mode,
                const std::vector<dnnl_accumulation_mode_t> &acc_mode,
                const std::vector<attr_t::deterministic_t> &deterministic) {
            for_(const auto &s : scales)
            for_(const auto &zp : zero_points)
            for_(const auto &po : post_ops)
            for_(const auto &sm : scratchpad_mode)
            for_(const auto &fm : fpmath_mode)
            for_(const auto &am : acc_mode)
            for (const auto &d : deterministic) {
                attrs_.push_back(get_attr(s, zp, po, sm, fm, am, d));
            }
        }

        using vector_type = std::vector<attr_t>;
        using iterator = vector_type::iterator;
        using const_iterator = vector_type::const_iterator;

        iterator begin() noexcept { return attrs_.begin(); }
        const_iterator begin() const noexcept { return attrs_.begin(); }
        iterator end() noexcept { return attrs_.end(); }
        const_iterator end() const noexcept { return attrs_.end(); }

    private:
        std::vector<attr_t> attrs_;
    };

    base_settings_t() {
        dnnl_get_default_fpmath_mode(&(this->fpmath_mode[0].mode));
    };

    // Parsed members
    std::vector<int64_t> mb {0};
    std::vector<bool> inplace {false};
    std::vector<attr_t::arg_scales_t> scales {attr_t::arg_scales_t()};
    std::vector<attr_t::zero_points_t> zero_points {attr_t::zero_points_t()};
    std::vector<attr_t::post_ops_t> post_ops {attr_t::post_ops_t()};
    std::vector<dnnl_scratchpad_mode_t> scratchpad_mode {
            attr_t::get_default_scratchpad_mode()};
    std::vector<attr_t::fpmath_mode_t> fpmath_mode {attr_t::fpmath_mode_t()};
    std::vector<dnnl_accumulation_mode_t> acc_mode {
            dnnl_accumulation_mode_strict};
    std::vector<attr_t::deterministic_t> deterministic {
            attr_t::deterministic_t()};
    std::vector<thr_ctx_t> ctx_init {default_thr_ctx};
    std::vector<thr_ctx_t> ctx_exe {default_thr_ctx};
    const char *pattern = NULL;
    // Non-parsed members
    settings_attributes_t attributes;

    const char *perf_template_csv_base(const std::string &driver_args) const {
        static const std::string csv_pre
                = std::string("perf,%engine%,%impl%,%name%,");
        static const std::string csv_post = std::string(
                ",%attr%,%DESC%,%Gops%,%+ctime%,%-time%,%-Gflops%,%0time%,%"
                "0Gflops%");
        static const std::string csv = csv_pre + driver_args + csv_post;
        return csv.c_str();
    }

    const char *perf_template_def
            = "perf,%engine%,%impl%,%name%,%prb%,%Gops%,%+ctime%,%-time%,%-"
              "Gflops%,%0time%,%0Gflops%";
    const char *perf_template = perf_template_def;

    // TODO: move to settings_attributes_t
    template <typename... ArgsT>
    static attr_t get_attr(const ArgsT &...args) {
        attr_t attr;
        attr.insert(args...);
        return attr;
    }

    // Returns `true` if all vector members in this class have capacity of one.
    virtual bool has_single_setup() const {
        return mb.size() == 1 && inplace.size() == 1 && scales.size() == 1
                && zero_points.size() == 1 && post_ops.size() == 1
                && scratchpad_mode.size() == 1 && fpmath_mode.size() == 1
                && acc_mode.size() == 1 && deterministic.size() == 1
                && ctx_init.size() == 1 && ctx_exe.size() == 1;
    }

    virtual void finalize() {
        attributes.clear();
        attributes.init(scales, zero_points, post_ops, scratchpad_mode,
                fpmath_mode, acc_mode, deterministic);
    }
};

#endif
