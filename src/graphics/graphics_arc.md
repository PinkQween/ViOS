**VIX Vios Graphx API**

┌─────────────────────────────────────────────────────────────┐
│                    USER APPLICATIONS                        │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │   Ponggg    │  │ VIX Demo    │  │  Other stuffff      │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────▼─────────┐
                    │   VIX Graphics    │
                    │   User Library    │
                    │   (vix_*.h/.c)    │
                    └─────────┬─────────┘
                              │ System Calls (int 0x80)
┌─────────────────────────────▼─────────────────────────────────┐
│                    KERNEL SPACE                               │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │           VIX System Call Handlers                      │  │
│  │        (src/isr80h/vix_graphics.c)                      │  │
│  │                                                         │  │
│  │  • isr80h_command11_vix_draw_pixel()                    │  │
│  │  • isr80h_command12_vix_draw_rect()                     │  │
│  │  • isr80h_command13_vix_fill_rect()                     │  │
│  │  • isr80h_command14_vix_clear_screen()                  │  │
│  │  • isr80h_command15_vix_present_frame()                 │  │
│  │  • etc...                                               │  │
│  └─────────────────┬───────────────────────────────────────┘  │
│                    │ Direct API calls                         │
│  ┌─────────────────▼───────────────────────────────────────┐  │
│  │         EXISTING ViOS Graphics System                   │  │
│  │         (src/graphics/graphics.c)                       │  │
│  │                                                         │  │
│  │  • graphics_set_pixel()                                 │  │
│  │  • graphics_fill_rect()                                 │  │
│  │  • graphics_clear_surface()                             │  │
│  │  • graphics_present()                                   │  │
│  │  • GraphicsContext management                           │  │
│  │  • Double buffering                                     │  │
│  │  • Color conversion                                     │  │
│  └─────────────────┬───────────────────────────────────────┘  │
│                    │                                          │
│  ┌─────────────────▼───────────────────────────────────────┐  │
│  │            Hardware Abstraction                         │  │
│  │                                                         │  │
│  │  • VBE framebuffer access                               │  │
│  │  • Memory mapping                                       │  │
│  │  • Pixel format conversion                              │  │
│  │  • Hardware-specific optimizations                      │  │
│  └─────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────--───┘