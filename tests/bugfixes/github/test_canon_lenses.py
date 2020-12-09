# -*- coding: utf-8 -*-

import collections
import re

import system_tests


# List of all supported lenses, duplicated from canonCsLensType in src/canonmn_int.cpp,
# to ensure that lenses do not get removed accidentially.
# Lenses should never be removed from this list to guarantee backwards-compatibility.
lenses = '''
        {    1, "Canon EF 50mm f/1.8"                                       },
        {    2, "Canon EF 28mm f/2.8"                                       },
        {    2, "Sigma 24mm f/2.8 Super Wide II"                            }, // 1
        {    3, "Canon EF 135mm f/2.8 Soft"                                 },
        {    4, "Canon EF 35-105mm f/3.5-4.5"                               },
        {    4, "Sigma UC Zoom 35-135mm f/4-5.6"                            }, // 1
        {    5, "Canon EF 35-70mm f/3.5-4.5"                                },
        {    6, "Canon EF 28-70mm f/3.5-4.5"                                },
        {    6, "Sigma 18-50mm f/3.5-5.6 DC"                                }, // 1
        {    6, "Sigma 18-125mm f/3.5-5.6 DC IF ASP"                        }, // 2
        {    6, "Tokina AF 193-2 19-35mm f/3.5-4.5"                         }, // 3
        {    6, "Sigma 28-80mm f/3.5-5.6 II Macro"                          }, // 4
        {    6, "Sigma 28-300mm f/3.5-6.3 DG Macro"                         }, // 5
        {    7, "Canon EF 100-300mm f/5.6L"                                 },
        {    8, "Canon EF 100-300mm f/5.6"                                  },
        {    8, "Sigma 70-300mm f/4-5.6 [APO] DG Macro"                     }, // 1
        {    8, "Tokina AT-X 242 AF 24-200mm f/3.5-5.6"                     }, // 2
        {    9, "Canon EF 70-210mm f/4"                                     },
        {    9, "Sigma 55-200mm f/4-5.6 DC"                                 }, // 1
        {   10, "Canon EF 50mm f/2.5 Macro"                                 },
        {   10, "Sigma 50mm f/2.8 EX"                                       }, // 1
        {   10, "Sigma 28mm f/1.8"                                          }, // 2
        {   10, "Sigma 105mm f/2.8 Macro EX"                                }, // 3
        {   10, "Sigma 70mm f/2.8 EX DG Macro EF"                           }, // 4
        {   11, "Canon EF 35mm f/2"                                         },
        {   13, "Canon EF 15mm f/2.8 Fisheye"                               },
        {   14, "Canon EF 50-200mm f/3.5-4.5L"                              },
        {   15, "Canon EF 50-200mm f/3.5-4.5"                               },
        {   16, "Canon EF 35-135mm f/3.5-4.5"                               },
        {   17, "Canon EF 35-70mm f/3.5-4.5A"                               },
        {   18, "Canon EF 28-70mm f/3.5-4.5"                                },
        {   20, "Canon EF 100-200mm f/4.5A"                                 },
        {   21, "Canon EF 80-200mm f/2.8L"                                  },
        {   22, "Canon EF 20-35mm f/2.8L"                                   },
        {   22, "Tokina AT-X 280 AF Pro 28-80mm f/2.8 Aspherical"           }, // 1
        {   23, "Canon EF 35-105mm f/3.5-4.5"                               },
        {   24, "Canon EF 35-80mm f/4-5.6 Power Zoom"                       },
        {   25, "Canon EF 35-80mm f/4-5.6 Power Zoom"                       },
        {   26, "Canon EF 100mm f/2.8 Macro"                                },
        {   26, "Cosina 100mm f/3.5 Macro AF"                               }, // 1
        {   26, "Tamron SP AF 90mm f/2.8 Di Macro"                          }, // 2
        {   26, "Tamron SP AF 180mm f/3.5 Di Macro"                         }, // 3
        {   26, "Carl Zeiss Planar T* 50mm f/1.4"                           }, // 4
        {   26, "Voigtlander APO Lanthar 125mm f/2.5 SL Macro"              }, // 5
        {   26, "Carl Zeiss Planar T 85mm f/1.4 ZE"                         }, // 6
        {   27, "Canon EF 35-80mm f/4-5.6"                                  },
        {   28, "Canon EF 80-200mm f/4.5-5.6"                               },
        {   28, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"              }, // 1
        {   28, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro" }, // 2
        {   28, "Tamron AF 70-300mm f/4-5.6 Di LD 1:2 Macro"                }, // 3
        {   28, "Tamron AF Aspherical 28-200mm f/3.8-5.6"                   }, // 4
        {   29, "Canon EF 50mm f/1.8 II"                                    },
        {   30, "Canon EF 35-105mm f/4.5-5.6"                               },
        {   31, "Canon EF 75-300mm f/4-5.6"                                 },
        {   31, "Tamron SP AF 300mm f/2.8 LD IF"                            }, // 1
        {   32, "Canon EF 24mm f/2.8"                                       },
        {   32, "Sigma 15mm f/2.8 EX Fisheye"                               }, // 1
        {   33, "Voigtlander Ultron 40mm f/2 SLII Aspherical"               },
        {   33, "Voigtlander Color Skopar 20mm f/3.5 SLII Aspherical"       }, // 1
        {   33, "Voigtlander APO-Lanthar 90mm f/3.5 SLII Close Focus"       }, // 2
        {   33, "Carl Zeiss Distagon T* 15mm f/2.8 ZE"                      }, // 3
        {   33, "Carl Zeiss Distagon T* 18mm f/3.5 ZE"                      }, // 4
        {   33, "Carl Zeiss Distagon T* 21mm f/2.8 ZE"                      }, // 5
        {   33, "Carl Zeiss Distagon T* 25mm f/2 ZE"                        }, // 6
        {   33, "Carl Zeiss Distagon T* 28mm f/2 ZE"                        }, // 7
        {   33, "Carl Zeiss Distagon T* 35mm f/2 ZE"                        }, // 8
        {   33, "Carl Zeiss Distagon T* 35mm f/1.4 ZE"                      }, // 9
        {   33, "Carl Zeiss Planar T* 50mm f/1.4 ZE"                        }, // 10
        {   33, "Carl Zeiss Makro-Planar T* 50mm f/2 ZE"                    }, // 11
        {   33, "Carl Zeiss Makro-Planar T* 100mm f/2 ZE"                   }, // 12
        {   33, "Carl Zeiss Apo-Sonnar T* 135mm f/2 ZE"                     }, // 13
        {   35, "Canon EF 35-80mm f/4-5.6"                                  },
        {   36, "Canon EF 38-76mm f/4.5-5.6"                                },
        {   37, "Canon EF 35-80mm f/4-5.6"                                  },
        {   37, "Tamron 70-200mm f/2.8 Di LD IF Macro"                      }, // 1
        {   37, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro"}, // 2
        {   37, "Tamron SP AF 17-50mm f/2.8 XR Di II VC LD Aspherical [IF]" }, // 3
        {   37, "Tamron AF 18-270mm f/3.5-6.3 Di II VC LD Aspherical [IF] Macro"}, // 4
        {   38, "Canon EF 80-200mm f/4.5-5.6"                               },
        {   39, "Canon EF 75-300mm f/4-5.6"                                 },
        {   40, "Canon EF 28-80mm f/3.5-5.6"                                },
        {   41, "Canon EF 28-90mm f/4-5.6"                                  },
        {   42, "Canon EF 28-200mm f/3.5-5.6"                               },
        {   42, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro"}, // 1
        {   43, "Canon EF 28-105mm f/4-5.6"                                 },
        {   44, "Canon EF 90-300mm f/4.5-5.6"                               },
        {   45, "Canon EF-S 18-55mm f/3.5-5.6 [II]"                         },
        {   46, "Canon EF 28-90mm f/4-5.6"                                  },
        {   47, "Zeiss Milvus 35mm f/2"                                     },
        {   47, "Zeiss Milvus 50mm f/2 Makro"                               }, // 1
        {   47, "Zeiss Milvus 135mm f/2 ZE"                                 }, // 2
        {   48, "Canon EF-S 18-55mm f/3.5-5.6 IS"                           },
        {   49, "Canon EF-S 55-250mm f/4-5.6 IS"                            },
        {   50, "Canon EF-S 18-200mm f/3.5-5.6 IS"                          },
        {   51, "Canon EF-S 18-135mm f/3.5-5.6 IS"                          },
        {   52, "Canon EF-S 18-55mm f/3.5-5.6 IS II"                        },
        {   53, "Canon EF-S 18-55mm f/3.5-5.6 III"                          },
        {   54, "Canon EF-S 55-250mm f/4-5.6 IS II"                         },
        {   60, "Irix 11mm f/4"                                             },
        {   80, "Canon TS-E 50mm f/2.8L Macro"                              },
        {   81, "Canon TS-E 90mm f/2.8L Macro"                              },
        {   82, "Canon TS-E 135mm f/4L Macro"                               },
        {   94, "Canon TS-E 17mm f/4L"                                      },
        {   95, "Canon TS-E 24mm f/3.5L II"                                 },
        {  103, "Samyang AF 14mm f/2.8 EF"                                  },
        {  106, "Rokinon SP / Samyang XP 35mm f/1.2"                        },
        {  112, "Sigma 28mm f/1.5 FF High-speed Prime"                      },
        {  112, "Sigma 40mm f/1.5 FF High-speed Prime"                      }, // 1
        {  112, "Sigma 105mm f/1.5 FF High-speed Prime"                     }, // 2
        {  117, "Tamron 35-150mm f/2.8-4.0 Di VC OSD"                       },
        {  117, "Tamron SP 35mm f/1.4 Di USD"                               }, // 1
        {  124, "Canon MP-E 65mm f/2.8 1-5x Macro Photo"                    },
        {  125, "Canon TS-E 24mm f/3.5L"                                    },
        {  126, "Canon TS-E 45mm f/2.8"                                     },
        {  127, "Canon TS-E 90mm f/2.8"                                     },
        {  127, "Tamron 18-200mm f/3.5-6.3 Di II VC"                        }, // 1
        {  129, "Canon EF 300mm f/2.8L USM"                                 },
        {  130, "Canon EF 50mm f/1.0L USM"                                  },
        {  131, "Canon EF 28-80mm f/2.8-4L USM"                             },
        {  131, "Sigma 8mm f/3.5 EX DG Circular Fisheye"                    }, // 1
        {  131, "Sigma 17-35mm f/2.8-4 EX DG Aspherical HSM"                }, // 2
        {  131, "Sigma 17-70mm f/2.8-4.5 DC Macro"                          }, // 3
        {  131, "Sigma APO 50-150mm f/2.8 [II] EX DC HSM"                   }, // 4
        {  131, "Sigma APO 120-300mm f/2.8 EX DG HSM"                       }, // 5
        {  131, "Sigma 70-200mm f/2.8 APO EX HSM"                           }, // 6
        {  131, "Sigma 28-70mm f/2.8-4 DG"                                  }, // 7
        {  132, "Canon EF 1200mm f/5.6L USM"                                },
        {  134, "Canon EF 600mm f/4L IS USM"                                },
        {  135, "Canon EF 200mm f/1.8L USM"                                 },
        {  136, "Canon EF 300mm f/2.8L USM"                                 },
        {  136, "Tamron SP 15-30mm f/2.8 Di VC USD"                         }, // 1
        {  137, "Canon EF 85mm f/1.2L USM"                                  },
        {  137, "Sigma 18-50mm f/2.8-4.5 DC OS HSM"                         }, // 1
        {  137, "Sigma 50-200mm f/4-5.6 DC OS HSM"                          }, // 2
        {  137, "Sigma 18-250mm f/3.5-6.3 DC OS HSM"                        }, // 3
        {  137, "Sigma 24-70mm f/2.8 IF EX DG HSM"                          }, // 4
        {  137, "Sigma 18-125mm f/3.8-5.6 DC OS HSM"                        }, // 5
        {  137, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM | C"                 }, // 6
        {  137, "Sigma 17-50mm f/2.8 OS HSM"                                }, // 7
        {  137, "Sigma 18-200mm f/3.5-6.3 DC OS HSM [II]"                   }, // 8
        {  137, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"                 }, // 9
        {  137, "Sigma 8-16mm f/4.5-5.6 DC HSM"                             }, // 10
        {  137, "Tamron SP 60mm f/2 Macro Di II"                            }, // 11
        {  137, "Sigma 10-20mm f/3.5 EX DC HSM"                             }, // 12
        {  137, "Sigma 18-35mm f/1.8 DC HSM | A"                            }, // 13
        {  137, "Sigma 12-24mm f/4.5-5.6 DG HSM II"                         }, // 14
        {  137, "Sigma 70-300mm f/4-5.6 DG OS"                              }, // 15
        {  138, "Canon EF 28-80mm f/2.8-4L"                                 },
        {  139, "Canon EF 400mm f/2.8L USM"                                 },
        {  140, "Canon EF 500mm f/4.5L USM"                                 },
        {  141, "Canon EF 500mm f/4.5L USM"                                 },
        {  142, "Canon EF 300mm f/2.8L IS USM"                              },
        {  143, "Canon EF 500mm f/4L IS USM"                                },
        {  143, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM"                     }, // 1
        {  143, "Sigma 24-105mm f/4 DG OS HSM | A"                          }, // 2
        {  144, "Canon EF 35-135mm f/4-5.6 USM"                             },
        {  145, "Canon EF 100-300mm f/4.5-5.6 USM"                          },
        {  146, "Canon EF 70-210mm f/3.5-4.5 USM"                           },
        {  147, "Canon EF 35-135mm f/4-5.6 USM"                             },
        {  148, "Canon EF 28-80mm f/3.5-5.6 USM"                            },
        {  149, "Canon EF 100mm f/2 USM"                                    },
        {  150, "Canon EF 14mm f/2.8L USM"                                  },
        {  150, "Sigma 20mm EX f/1.8"                                       }, // 1
        {  150, "Sigma 30mm f/1.4 DC HSM"                                   }, // 2
        {  150, "Sigma 24mm f/1.8 DG Macro EX"                              }, // 3
        {  150, "Sigma 28mm f/1.8 DG Macro EX"                              }, // 4
        {  150, "Sigma 18-35mm f/1.8 DC HSM | A"                            }, // 5
        {  151, "Canon EF 200mm f/2.8L USM"                                 },
        {  152, "Canon EF 300mm f/4L IS USM"                                },
        {  152, "Sigma 12-24mm f/4.5-5.6 EX DG ASPHERICAL HSM"              }, // 1
        {  152, "Sigma 14mm f/2.8 EX Aspherical HSM"                        }, // 2
        {  152, "Sigma 10-20mm f/4-5.6"                                     }, // 3
        {  152, "Sigma 100-300mm f/4"                                       }, // 4
        {  152, "Sigma 300-800mm f/5.6 APO EX DG HSM"                       }, // 5
        {  153, "Canon EF 35-350mm f/3.5-5.6L USM"                          },
        {  153, "Sigma 50-500mm f/4-6.3 APO HSM EX"                         }, // 1
        {  153, "Tamron AF 28-300mm f/3.5-6.3 XR LD Aspherical [IF] Macro"  }, // 2
        {  153, "Tamron AF 18-200mm f/3.5-6.3 XR Di II LD Aspherical [IF] Macro"}, // 3
        {  153, "Tamron 18-250mm f/3.5-6.3 Di II LD Aspherical [IF] Macro"  }, // 4
        {  154, "Canon EF 20mm f/2.8 USM"                                   },
        {  154, "Zeiss Milvus 21mm f/2.8"                                   }, // 1
        {  154, "Zeiss Milvus 15mm f/2.8 ZE"                                }, // 2
        {  154, "Zeiss Milvus 18mm f/2.8 ZE"                                }, // 3
        {  155, "Canon EF 85mm f/1.8 USM"                                   },
        {  155, "Sigma 14mm f/1.8 DG HSM | A"                               }, // 1
        {  156, "Canon EF 28-105mm f/3.5-4.5 USM"                           },
        {  156, "Tamron SP 70-300mm f/4-5.6 Di VC USD"                      }, // 1
        {  156, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"              }, // 2
        {  160, "Canon EF 20-35mm f/3.5-4.5 USM"                            },
        {  160, "Tamron AF 19-35mm f/3.5-4.5"                               }, // 1
        {  160, "Tokina AT-X 124 AF Pro DX 12-24mm f/4"                     }, // 2
        {  160, "Tokina AT-X 107 AF DX 10-17mm f/3.5-4.5 Fisheye"           }, // 3
        {  160, "Tokina AT-X 116 AF Pro DX 11-16mm f/2.8"                   }, // 4
        {  160, "Tokina AT-X 11-20 F2.8 PRO DX Aspherical 11-20mm f/2.8"    }, // 5
        {  161, "Canon EF 28-70mm f/2.8L USM"                               },
        {  161, "Sigma 24-70mm f/2.8 EX"                                    }, // 1
        {  161, "Sigma 24-60mm f/2.8 EX DG"                                 }, // 2
        {  161, "Tamron AF 17-50mm f/2.8 Di-II LD Aspherical"               }, // 3
        {  161, "Tamron 90mm f/2.8"                                         }, // 4
        {  161, "Tamron SP AF 17-35mm f/2.8-4 Di LD Aspherical IF"          }, // 5
        {  161, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro" }, // 6
        {  162, "Canon EF 200mm f/2.8L USM"                                 },
        {  163, "Canon EF 300mm f/4L"                                       },
        {  164, "Canon EF 400mm f/5.6L"                                     },
        {  165, "Canon EF 70-200mm f/2.8L USM"                              },
        {  166, "Canon EF 70-200mm f/2.8L USM + 1.4x"                       },
        {  167, "Canon EF 70-200mm f/2.8L USM + 2x"                         },
        {  168, "Canon EF 28mm f/1.8 USM"                                   },
        {  168, "Sigma 50-100mm f/1.8 DC HSM | A"                           }, // 1
        {  169, "Canon EF 17-35mm f/2.8L USM"                               },
        {  169, "Sigma 18-200mm f/3.5-6.3 DC OS"                            }, // 1
        {  169, "Sigma 15-30mm f/3.5-4.5 EX DG Aspherical"                  }, // 2
        {  169, "Sigma 18-50mm f/2.8 Macro"                                 }, // 3
        {  169, "Sigma 50mm f/1.4 EX DG HSM"                                }, // 4
        {  169, "Sigma 85mm f/1.4 EX DG HSM"                                }, // 5
        {  169, "Sigma 30mm f/1.4 EX DC HSM"                                }, // 6
        {  169, "Sigma 35mm f/1.4 DG HSM"                                   }, // 7
        {  169, "Sigma 70mm f/2.8 Macro EX DG"                              }, // 8
        {  170, "Canon EF 200mm f/2.8L II USM"                              },
        {  170, "Sigma 300mm f/2.8 APO EX DG HSM"                           }, // 1
        {  170, "Sigma 800mm f/5.6 APO EX DG HSM"                           }, // 2
        {  171, "Canon EF 300mm f/4L USM"                                   },
        {  172, "Canon EF 400mm f/5.6L USM"                                 },
        {  172, "Sigma 150-600mm f/5-6.3 DG OS HSM | S"                     }, // 1
        {  172, "Sigma 500mm f/4.5 APO EX DG HSM"                           }, // 2
        {  173, "Canon EF 180mm Macro f/3.5L USM"                           },
        {  173, "Sigma APO Macro 150mm f/2.8 EX DG HSM"                     }, // 1
        {  173, "Sigma 10mm f/2.8 EX DC Fisheye"                            }, // 2
        {  173, "Sigma 15mm f/2.8 EX DG Diagonal Fisheye"                   }, // 3
        {  173, "Venus Laowa 100mm f/2.8 2X Ultra Macro APO"                }, // 4
        {  173, "Sigma 150-500mm f/5-6.3 APO DG OS HSM + 2x"                }, // 5
        {  174, "Canon EF 135mm f/2L USM"                                   },
        {  174, "Sigma 70-200mm f/2.8 EX DG APO OS HSM"                     }, // 1
        {  174, "Sigma 50-500mm f/4.5-6.3 APO DG OS HSM"                    }, // 2
        {  174, "Sigma 150-500mm f/5-6.3 APO DG OS HSM"                     }, // 3
        {  174, "Zeiss Milvus 100mm f/2 Makro"                              }, // 4
        {  174, "Sigma APO 50-150mm f/2.8 EX DC OS HSM"                     }, // 5
        {  174, "Sigma APO 120-300mm f/2.8 EX DG OS HSM"                    }, // 6
        {  174, "Sigma 120-400mm f/4.5-5.6 APO DG OS HSM"                   }, // 7
        {  174, "Sigma 200-500mm f/2.8 APO EX DG"                           }, // 8
        {  175, "Canon EF 400mm f/2.8L USM"                                 },
        {  176, "Canon EF 24-85mm f/3.5-4.5 USM"                            },
        {  177, "Canon EF 300mm f/4L IS USM"                                },
        {  178, "Canon EF 28-135mm f/3.5-5.6 IS"                            },
        {  179, "Canon EF 24mm f/1.4L USM"                                  },
        {  180, "Canon EF 35mm f/1.4L USM"                                  },
        {  180, "Sigma 50mm f/1.4 DG HSM | A"                               }, // 1
        {  180, "Sigma 24mm f/1.4 DG HSM | A"                               }, // 2
        {  180, "Zeiss Milvus 85mm f/1.4"                                   }, // 3
        {  180, "Zeiss Otus 28mm f/1.4 ZE"                                  }, // 4
        {  180, "Sigma 20mm f/1.4 DG HSM | A"                               }, // 5
        {  181, "Canon EF 100-400mm f/4.5-5.6L IS USM + 1.4x"               },
        {  181, "Sigma 150-600mm f/5-6.3 DG OS HSM | S + 1.4x"              }, // 1
        {  182, "Canon EF 100-400mm f/4.5-5.6L IS USM + 2x"                 },
        {  182, "Sigma 150-600mm f/5-6.3 DG OS HSM | S + 2x"                }, // 1
        {  183, "Canon EF 100-400mm f/4.5-5.6L IS USM"                      },
        {  183, "Sigma 150mm f/2.8 EX DG OS HSM APO Macro"                  }, // 1
        {  183, "Sigma 105mm f/2.8 EX DG OS HSM Macro"                      }, // 2
        {  183, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"                  }, // 3
        {  183, "Sigma 150-600mm f/5-6.3 DG OS HSM | C"                     }, // 4
        {  183, "Sigma 100-400mm f/5-6.3 DG OS HSM"                         }, // 5
        {  183, "Sigma 180mm f/3.5 APO Macro EX DG IF HSM"                  }, // 6
        {  184, "Canon EF 400mm f/2.8L USM + 2x"                            },
        {  185, "Canon EF 600mm f/4L IS USM"                                },
        {  186, "Canon EF 70-200mm f/4L USM"                                },
        {  187, "Canon EF 70-200mm f/4L USM + 1.4x"                         },
        {  188, "Canon EF 70-200mm f/4L USM + 2x"                           },
        {  189, "Canon EF 70-200mm f/4L USM + 2.8x"                         },
        {  190, "Canon EF 100mm f/2.8 Macro USM"                            },
        {  191, "Canon EF 400mm f/4 DO IS"                                  },
        {  191, "Sigma 500mm f/4 DG OS HSM"                                 }, // 1
        {  193, "Canon EF 35-80mm f/4-5.6 USM"                              },
        {  194, "Canon EF 80-200mm f/4.5-5.6 USM"                           },
        {  195, "Canon EF 35-105mm f/4.5-5.6 USM"                           },
        {  196, "Canon EF 75-300mm f/4-5.6 USM"                             },
        {  197, "Canon EF 75-300mm f/4-5.6 IS USM"                          },
        {  197, "Sigma 18-300mm f/3.5-6.3 DC Macro OS HSM"                  }, // 1
        {  198, "Canon EF 50mm f/1.4 USM"                                   },
        {  198, "Zeiss Otus 55mm f/1.4 ZE"                                  }, // 1
        {  198, "Zeiss Otus 85mm f/1.4 ZE"                                  }, // 2
        {  198, "Zeiss Milvus 25mm f/1.4"                                   }, // 3
        {  198, "Zeiss Otus 100mm f/1.4"                                    }, // 4
        {  198, "Zeiss Milvus 35mm f/1.4 ZE"                                }, // 5
        {  198, "Yongnuo YN 35mm f/2"                                       }, // 6
        {  199, "Canon EF 28-80mm f/3.5-5.6 USM"                            },
        {  200, "Canon EF 75-300mm f/4-5.6 USM"                             },
        {  201, "Canon EF 28-80mm f/3.5-5.6 USM"                            },
        {  202, "Canon EF 28-80mm f/3.5-5.6 USM IV"                         },
        {  208, "Canon EF 22-55mm f/4-5.6 USM"                              },
        {  209, "Canon EF 55-200mm f/4.5-5.6"                               },
        {  210, "Canon EF 28-90mm f/4-5.6 USM"                              },
        {  211, "Canon EF 28-200mm f/3.5-5.6 USM"                           },
        {  212, "Canon EF 28-105mm f/4-5.6 USM"                             },
        {  213, "Canon EF 90-300mm f/4.5-5.6 USM"                           },
        {  213, "Tamron SP 150-600mm f/5-6.3 Di VC USD"                     }, // 1
        {  213, "Tamron 16-300mm f/3.5-6.3 Di II VC PZD Macro"              }, // 2
        {  213, "Tamron SP 35mm f/1.8 Di VC USD"                            }, // 3
        {  213, "Tamron SP 45mm f/1.8 Di VC USD"                            }, // 4
        {  214, "Canon EF-S 18-55mm f/3.5-5.6 USM"                          },
        {  215, "Canon EF 55-200mm f/4.5-5.6 II USM"                        },
        {  217, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"                 },
        {  220, "Yongnuo YN 50mm f/1.8"                                     },
        {  224, "Canon EF 70-200mm f/2.8L IS USM"                           },
        {  225, "Canon EF 70-200mm f/2.8L IS USM + 1.4x"                    },
        {  226, "Canon EF 70-200mm f/2.8L IS USM + 2x"                      },
        {  227, "Canon EF 70-200mm f/2.8L IS USM + 2.8x"                    },
        {  228, "Canon EF 28-105mm f/3.5-4.5 USM"                           },
        {  229, "Canon EF 16-35mm f/2.8L USM"                               },
        {  230, "Canon EF 24-70mm f/2.8L USM"                               },
        {  231, "Canon EF 17-40mm f/4L USM"                                 },
        {  231, "Sigma 12-24mm f/4 DG HSM A016"                             }, // 1
        {  232, "Canon EF 70-300mm f/4.5-5.6 DO IS USM"                     },
        {  233, "Canon EF 28-300mm f/3.5-5.6L IS USM"                       },
        {  234, "Canon EF-S 17-85mm f/4-5.6 IS USM"                         },
        {  234, "Tokina AT-X 12-28 PRO DX 12-28mm f/4"                      }, // 1
        {  235, "Canon EF-S 10-22mm f/3.5-4.5 USM"                          },
        {  236, "Canon EF-S 60mm f/2.8 Macro USM"                           },
        {  237, "Canon EF 24-105mm f/4L IS USM"                             },
        {  238, "Canon EF 70-300mm f/4-5.6 IS USM"                          },
        {  239, "Canon EF 85mm f/1.2L II USM"                               },
        {  240, "Canon EF-S 17-55mm f/2.8 IS USM"                           },
        {  240, "Sigma 17-50mm f/2.8 EX DC OS HSM"                          }, // 1
        {  241, "Canon EF 50mm f/1.2L USM"                                  },
        {  242, "Canon EF 70-200mm f/4L IS USM"                             },
        {  243, "Canon EF 70-200mm f/4L IS USM + 1.4x"                      },
        {  244, "Canon EF 70-200mm f/4L IS USM + 2x"                        },
        {  245, "Canon EF 70-200mm f/4L IS USM + 2.8x"                      },
        {  246, "Canon EF 16-35mm f/2.8L II USM"                            },
        {  247, "Canon EF 14mm f/2.8L II USM"                               },
        {  248, "Canon EF 200mm f/2L IS USM"                                },
        {  248, "Sigma 24-35mm f/2 DG HSM | A"                              }, // 1
        {  248, "Sigma 135mm f/2 FF High-Speed Prime | 017"                 }, // 2
        {  248, "Sigma 24-35mm f/2.2 FF Zoom | 017"                         }, // 3
        {  248, "Sigma 135mm f/1.8 DG HSM A017"                             }, // 4
        {  249, "Canon EF 800mm f/5.6L IS USM"                              },
        {  250, "Canon EF 24mm f/1.4L II USM"                               },
        {  250, "Sigma 20mm f/1.4 DG HSM | A"                               }, // 1
        {  250, "Tokina Opera 16-28mm f/2.8 FF"                             }, // 2
        {  250, "Sigma 85mm f/1.4 DG HSM A016"                              }, // 3
        {  251, "Canon EF 70-200mm f/2.8L IS II USM"                        },
        {  252, "Canon EF 70-200mm f/2.8L IS II USM + 1.4x"                 },
        {  253, "Canon EF 70-200mm f/2.8L IS II USM + 2x"                   },
        {  254, "Canon EF 100mm f/2.8L Macro IS USM"                        }, // 1
        {  254, "Tamron SP 90mm f/2.8 Di VC USD Macro 1:1"                  }, // 2
        {  255, "Sigma 24-105mm f/4 DG OS HSM | A"                          },
        {  255, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"                  }, // 1
        {  368, "Sigma 14-24mm f/2.8 DG HSM | A"                            },
        {  368, "Sigma 20mm f/1.4 DG HSM | A"                               }, // 1
        {  368, "Sigma 50mm f/1.4 DG HSM | A"                               }, // 2
        {  368, "Sigma 40mm f/1.4 DG HSM | A"                               }, // 3
        {  368, "Sigma 60-600mm f/4.5-6.3 DG OS HSM | S"                    }, // 4
        {  368, "Sigma 28mm f/1.4 DG HSM | A"                               }, // 5
        {  368, "Sigma 150-600mm f/5-6.3 DG OS HSM | S"                     }, // 6
        {  368, "Sigma 85mm f/1.4 DG HSM | A"                               }, // 7
        {  368, "Sigma 105mm f/1.4 DG HSM"                                  }, // 8
        {  368, "Sigma 70mm f/2.8 DG Macro"                                 }, // 9
        {  368, "Sigma 18-35mm f/1.8 DC HSM | A"                            }, // 10
        {  488, "Canon EF-S 15-85mm f/3.5-5.6 IS USM"                       },
        {  489, "Canon EF 70-300mm f/4-5.6L IS USM"                         },
        {  490, "Canon EF 8-15mm f/4L Fisheye USM"                          },
        {  491, "Canon EF 300mm f/2.8L IS II USM"                           },
        {  491, "Tamron SP 70-200mm f/2.8 Di VC USD G2"                     }, // 1
        {  491, "Tamron 18-400mm f/3.5-6.3 Di II VC HLD"                    }, // 2
        {  491, "Tamron 100-400mm f/4.5-6.3 Di VC USD"                      }, // 3
        {  491, "Tamron 70-210mm f/4 Di VC USD"                             }, // 4
        {  491, "Tamron 70-210mm f/4 Di VC USD + 1.4x"                      }, // 5
        {  491, "Tamron SP 24-70mm f/2.8 Di VC USD G2"                      }, // 6
        {  492, "Canon EF 400mm f/2.8L IS II USM"                           },
        {  493, "Canon EF 500mm f/4L IS II USM"                             },
        {  493, "Canon EF 24-105mm f/4L IS USM"                             }, // 1
        {  494, "Canon EF 600mm f/4L IS II USM"                             },
        {  495, "Canon EF 24-70mm f/2.8L II USM"                            },
        {  496, "Canon EF 200-400mm f/4L IS USM"                            },
        {  499, "Canon EF 200-400mm f/4L IS USM + 1.4x"                     },
        {  502, "Canon EF 28mm f/2.8 IS USM"                                },
        {  502, "Tamron 35mm f/1.8 Di VC USD"                               }, // 1
        {  503, "Canon EF 24mm f/2.8 IS USM"                                },
        {  504, "Canon EF 24-70mm f/4L IS USM"                              },
        {  505, "Canon EF 35mm f/2 IS USM"                                  },
        {  506, "Canon EF 400mm f/4 DO IS II USM"                           },
        {  507, "Canon EF 16-35mm f/4L IS USM"                              },
        {  508, "Canon EF 11-24mm f/4L USM"                                 },
        {  508, "Tamron 10-24mm f/3.5-4.5 Di II VC HLD"                     }, // 1
        {  624, "Sigma 70-200mm f/2.8 DG OS HSM | S"                        },
        {  747, "Canon EF 100-400mm f/4.5-5.6L IS II USM"                   },
        {  747, "Tamron SP 150-600mm f/5-6.3 Di VC USD G2"                  }, // 1
        {  748, "Canon EF 100-400mm f/4.5-5.6L IS II USM + 1.4x"            },
        {  748, "Tamron 70-210mm f/4 Di VC USD + 2x"                        }, // 1
        {  749, "Tamron 100-400mm f/4.5-6.3 Di VC USD + 2x"                 },
        {  750, "Canon EF 35mm f/1.4L II USM"                               },
        {  750, "Tamron SP 85mm f/1.8 Di VC USD"                            }, // 1
        {  750, "Tamron SP 45mm f/1.8 Di VC USD"                            }, // 2
        {  751, "Canon EF 16-35mm f/2.8L III USM"                           },
        {  752, "Canon EF 24-105mm f/4L IS II USM"                          },
        {  753, "Canon EF 85mm f/1.4L IS USM"                               },
        {  754, "Canon EF 70-200mm f/4L IS II USM"                          },
        {  757, "Canon EF 400mm f/2.8L IS III USM"                          },
        {  758, "Canon EF 600mm f/4L IS III USM"                            },
        { 1136, "Sigma 24-70mm f/2.8 DG OS HSM | Art 017"                   },
        { 4142, "Canon EF-S 18-135mm f/3.5-5.6 IS STM"                      },
        { 4143, "Canon EF-M 18-55mm f/3.5-5.6 IS STM"                       },
        { 4143, "Tamron 18-200mm f/3.5-6.3 Di III VC"                       }, // 1
        { 4144, "Canon EF 40mm f/2.8 STM"                                   },
        { 4145, "Canon EF-M 22mm f/2 STM"                                   },
        { 4146, "Canon EF-S 18-55mm f/3.5-5.6 IS STM"                       },
        { 4147, "Canon EF-M 11-22mm f/4-5.6 IS STM"                         },
        { 4148, "Canon EF-S 55-250mm f/4-5.6 IS STM"                        },
        { 4149, "Canon EF-M 55-200mm f/4.5-6.3 IS STM"                      },
        { 4150, "Canon EF-S 10-18mm f/4.5-5.6 IS STM"                       },
        { 4152, "Canon EF 24-105mm f/3.5-5.6 IS STM"                        },
        { 4153, "Canon EF-M 15-45mm f/3.5-6.3 IS STM"                       },
        { 4154, "Canon EF-S 24mm f/2.8 STM"                                 },
        { 4155, "Canon EF-M 28mm f/3.5 Macro IS STM"                        },
        { 4156, "Canon EF 50mm f/1.8 STM"                                   },
        { 4157, "Canon EF-M 18-150mm f/3.5-6.3 IS STM"                      },
        { 4158, "Canon EF-S 18-55mm f/4-5.6 IS STM"                         },
        { 4159, "Canon EF-M 32mm f/1.4 STM"                                 },
        { 4160, "Canon EF-S 35mm f/2.8 Macro IS STM"                        },
        { 4208, "Sigma 56mm f/1.4 DC DN | C"                                },
        {36910, "Canon EF 70-300mm f/4-5.6 IS II USM"                       },
        {36912, "Canon EF-S 18-135mm f/3.5-5.6 IS USM"                      },
        {61491, "Canon CN-E 14mm T3.1 L F"                                  },
        {61492, "Canon CN-E 24mm T1.5 L F"                                  },
        {61494, "Canon CN-E 85mm T1.3 L F"                                  },
        {61495, "Canon CN-E 135mm T2.2 L F"                                 },
        {61496, "Canon CN-E 35mm T1.5 L F"                                  },
        {61182, "Canon RF 35mm f/1.8 Macro IS STM"                          },
        {61182, "Canon RF 50mm f/1.2L USM"                                  }, // 1
        {61182, "Canon RF 24-105mm f/4L IS USM"                             }, // 2
        {61182, "Canon RF 28-70mm f/2L USM"                                 }, // 3
        {61182, "Canon RF 85mm f/1.2L USM"                                  }, // 4
        {61182, "Canon RF 24-240mm f/4-6.3 IS USM"                          }, // 5
        {61182, "Canon RF 24-70mm f/2.8L IS USM"                            }, // 6
        {61182, "Canon RF 15-35mm f/2.8L IS USM"                            }, // 7
'''

apertures = '1.0', '1.1', '1.2', '1.4', '1.6', '1.8', '2', '2.2', '2.5', '2.8', '3.2', '3.5', '4', '4.5', '5', '5.6', '6.3', '7.1', '8', '9', '10', '11', '13', '14', '16', '18', '20', '22', '25', '29', '32', '36', '40', '45'
fractions = {0: 0, 1: 12, 2: 20}
aperture_map = collections.defaultdict(lambda: '32000') # map unknown apertures to invalid value by default
aperture_map.update({value: (index // 3) * 32 + fractions[index % 3] for index, value in enumerate(apertures)})

for lens_match in re.finditer('(?P<lens_id>[0-9]+),.*"(?P<lens_description>.*)"', lenses):
    lens = lens_match.group('lens_description')

    metadata_match = re.search('((?P<focal_length_min>[0-9]+)-)?(?P<focal_length_max>[0-9]+)mm.*(f/|T)(?P<aperture>[0-9]+(\\.[0-9]+)?)[^+]*(\\+ (?P<tc>[0-9.]+)x)?', lens)
    if not metadata_match:
        raise ValueError('Invalid lens format: ' + lens)
    tc = float(metadata_match.group('tc') or 1)

    testname = lens_match.group('lens_id') + '_' + lens
    globals()[testname] = system_tests.CaseMeta('canon_lenses.' + testname, tuple(), {
        'filename': '$data_path/template.exv',
        'commands': ['$exiv2 -M"set Exif.CanonCs.LensType $lens_id" -M"set Exif.CanonCs.Lens $focal_length_max $focal_length_min 1" -M"set Exif.CanonCs.MaxAperture $aperture" $filename && $exiv2 -pa -K Exif.CanonCs.LensType $filename'],
        'stderr': [''],
        'stdout': ['Exif.CanonCs.LensType                        Short       1  $lens_description\n'],
        'retval': [0],
        **lens_match.groupdict(),
        'aperture': aperture_map[metadata_match.group('aperture')],
        'focal_length_min': int(int(metadata_match.group('focal_length_min') or metadata_match.group('focal_length_max')) * tc),
        'focal_length_max': int(int(metadata_match.group('focal_length_max')) * tc),
    })
