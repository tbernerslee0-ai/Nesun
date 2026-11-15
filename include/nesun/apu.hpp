#pragma once

#include <cstdint>
#include <array>

namespace Nesun {

class Bus;

class APU {
public:
    explicit APU(Bus& bus);
    ~APU() = default;

    // Lifecycle & Execution
    void reset();
    void step(); // Advances 1 CPU clock cycle

    // CPU-Mapped Register Interface ($4000 - $4017 + VTxx PCM $4030 - $4037)
    uint8_t cpu_read(uint16_t address);
    void cpu_write(uint16_t address, uint8_t value);

    // VTxx Audio Extension Control
    void set_vtxx_audio_enabled(bool enabled) { vtxx_audio_enabled_ = enabled; }
    bool is_vtxx_audio_enabled() const { return vtxx_audio_enabled_; }

    // Audio Output Sampling (Normalized -1.0f to 1.0f for SDL2 Audio Queue)
    float get_mixed_sample() const;
    bool is_sample_ready() const { return sample_ready_; }
    void clear_sample_ready() { sample_ready_ = false; }

    // Interrupt Signals
    bool frame_irq_requested = false;
    bool dmc_irq_requested = false;

private:
    Bus& bus_;
    bool vtxx_audio_enabled_ = false;

    // Timing & Sample Rate Generation
    uint64_t total_cycles_ = 0;
    double sample_accumulator_ = 0.0;
    bool sample_ready_ = false;
    static constexpr double CPU_CLOCK_NTSC = 1789773.0;
    static constexpr double SAMPLE_RATE = 44100.0;

    // Standard RP2A03 Channel State Structures
    struct PulseChannel {
        bool enabled = false;
        uint8_t duty = 0;
        uint8_t duty_step = 0;
        uint16_t timer = 0;
        uint16_t timer_reload = 0;
        uint8_t length_counter = 0;
        uint8_t volume = 0;
        bool constant_volume = false;
        bool halt_loop = false;
        uint8_t output() const;
    } pulse1_, pulse2_;

    struct TriangleChannel {
        bool enabled = false;
        uint16_t timer = 0;
        uint16_t timer_reload = 0;
        uint8_t step_index = 0;
        uint8_t length_counter = 0;
        uint8_t linear_counter = 0;
        uint8_t output() const;
    } triangle_;

    struct NoiseChannel {
        bool enabled = false;
        uint16_t shift_register = 1;
        uint16_t timer = 0;
        uint16_t timer_reload = 0;
        uint8_t length_counter = 0;
        uint8_t volume = 0;
        bool mode = false;
        uint8_t output() const;
    } noise_;

    struct DMCChannel {
        bool enabled = false;
        bool irq_enabled = false;
        bool loop = false;
        uint16_t sample_address = 0xC000;
        uint16_t current_address = 0xC000;
        uint16_t sample_length = 0;
        uint16_t bytes_remaining = 0;
        uint8_t output_level = 0;
    } dmc_;

    // VTxx / VTxxx Dual PCM Extension Channels ($4030 - $4037)
    struct VTxxPCMChannel {
        bool enabled = false;
        uint32_t start_address = 0;
        uint32_t current_address = 0;
        uint16_t length = 0;
        uint16_t frequency_div = 0;
        uint16_t timer = 0;
        uint8_t volume = 0;
        int8_t current_sample = 0;
    };
    std::array<VTxxPCMChannel, 2> vtxx_pcm_{};

    // Frame Counter ($4017)
    uint8_t frame_counter_mode_ = 0;
    bool irq_inhibit_ = false;
    uint32_t frame_sequencer_step_ = 0;

    // Internal Clocking Helpers
    void clock_frame_sequencer();
    void clock_quarter_frame();
    void clock_half_frame();
    void clock_vtxx_pcm();
};

} // namespace Nesun
