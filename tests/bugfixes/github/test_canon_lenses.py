# -*- coding: utf-8 -*-

import re

import system_tests


# List of all supported lenses, duplicated from canonCsLensType in src/canonmn_int.cpp,
# to ensure that lenses do not get removed accidentially.
# Lenses should never be removed from this list to guarantee backwards-compatibility.
lenses = '''
        {   1, "Canon EF 50mm f/1.8"                                        },
        {   2, "Canon EF 28mm f/2.8"                                        },
        {   3, "Canon EF 135mm f/2.8 Soft"                                  },
        {   4, "Canon EF 35-105mm f/3.5-4.5"                                }, // 0
        {   4, "Sigma UC Zoom 35-135mm f/4-5.6"                             }, // 1
        {   5, "Canon EF 35-70mm f/3.5-4.5"                                 },
        {   6, "Canon EF 28-70mm f/3.5-4.5"                                 }, // 0
        {   6, "Sigma 18-50mm f/3.5-5.6 DC"                                 }, // 1
        {   6, "Sigma 18-125mm f/3.5-5.6 DC IF ASP"                         }, // 2
        {   6, "Tokina AF193-2 19-35mm f/3.5-4.5"                           }, // 3
        {   6, "Sigma 28-80mm f/3.5-5.6 II Macro"                           }, // 4
        {   7, "Canon EF 100-300mm f/5.6L"                                  },
        {   8, "Canon EF 100-300mm f/5.6"                                   }, // 0
        {   8, "Sigma 70-300mm f/4-5.6 [APO] DG Macro"                      }, // 1
        {   8, "Tokina AT-X 242 AF 24-200mm f/3.5-5.6"                      }, // 2
        {   9, "Canon EF 70-210mm f/4"                                      }, // 0
        {   9, "Sigma 55-200mm f/4-5.6 DC"                                  }, // 1
        {  10, "Canon EF 50mm f/2.5 Macro"                                  }, // 0
        {  10, "Sigma 50mm f/2.8 EX"                                        }, // 1
        {  10, "Sigma 28mm f/1.8"                                           }, // 2
        {  10, "Sigma 105mm f/2.8 Macro EX"                                 }, // 3
        {  10, "Sigma 70mm f/2.8 EX DG Macro EF"                            }, // 4
        {  11, "Canon EF 35mm f/2"                                          },
        {  13, "Canon EF 15mm f/2.8 Fisheye"                                },
        {  14, "Canon EF 50-200mm f/3.5-4.5L"                               },
        {  15, "Canon EF 50-200mm f/3.5-4.5"                                },
        {  16, "Canon EF 35-135mm f/3.5-4.5"                                },
        {  17, "Canon EF 35-70mm f/3.5-4.5A"                                },
        {  18, "Canon EF 28-70mm f/3.5-4.5"                                 },
        {  20, "Canon EF 100-200mm f/4.5A"                                  },
        {  21, "Canon EF 80-200mm f/2.8L"                                   },
        {  22, "Canon EF 20-35mm f/2.8L"                                    }, // 0
        {  22, "Tokina AT-X 280 AF PRO 28-80mm f/2.8 Aspherical"            }, // 1
        {  23, "Canon EF 35-105mm f/3.5-4.5"                                },
        {  24, "Canon EF 35-80mm f/4-5.6 Power Zoom"                        },
        {  25, "Canon EF 35-80mm f/4-5.6 Power Zoom"                        },
        {  26, "Canon EF 100mm f/2.8 Macro"                                 }, // 0
        {  26, "Cosina 100mm f/3.5 Macro AF"                                }, // 1
        {  26, "Tamron SP AF 90mm f/2.8 Di Macro"                           }, // 2
        {  26, "Tamron SP AF 180mm f/3.5 Di Macro"                          }, // 3
        {  26, "Carl Zeiss Planar T* 50mm f/1.4"                            }, // 4
        {  27, "Canon EF 35-80mm f/4-5.6"                                   },
        {  28, "Canon EF 80-200mm f/4.5-5.6"                                }, // 0
        {  28, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"               }, // 1
        {  28, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro"  }, // 2
        {  28, "Tamron AF 70-300mm f/4-5.6 Di LD 1:2 Macro"                 }, // 3
        {  28, "Tamron AF Aspherical 28-200mm f/3.8-5.6"                    }, // 4
        {  29, "Canon EF 50mm f/1.8 II"                                     },
        {  30, "Canon EF 35-105mm f/4.5-5.6"                                },
        {  31, "Canon EF 75-300mm f/4-5.6"                                  }, // 0
        {  31, "Tamron SP AF 300mm f/2.8 LD IF"                             }, // 1
        {  32, "Canon EF 24mm f/2.8"                                        }, // 0
        {  32, "Sigma 15mm f/2.8 EX Fisheye"                                }, // 1
        {  33, "Voigtlander Ultron 40mm f/2 SLII Aspherical"                }, // 0
        {  33, "Voigtlander Color Skopar 20mm f/3.5 SLII Aspherical"        }, // 1
        {  33, "Voigtlander APO-Lanthar 90mm f/3.5 SLII Close Focus"        }, // 2
        {  33, "Carl Zeiss Distagon 15mm T* f/2.8 ZE"                       }, // 3
        {  33, "Carl Zeiss Distagon 18mm T* f/3.5 ZE"                       }, // 4
        {  33, "Carl Zeiss Distagon 21mm T* f/2.8 ZE"                       }, // 5
        {  33, "Carl Zeiss Distagon 25mm T* f/2 ZE"                         }, // 6
        {  33, "Carl Zeiss Distagon 28mm T* f/2 ZE"                         }, // 7
        {  33, "Carl Zeiss Distagon 35mm T* f/2 ZE"                         }, // 8
        {  33, "Carl Zeiss Distagon 35mm T* f/1.4 ZE"                       }, // 9
        {  33, "Carl Zeiss Planar 50mm T* f/1.4 ZE"                         }, // 10
        {  33, "Carl Zeiss Makro-Planar T* 50mm f/2 ZE"                     }, // 11
        {  33, "Carl Zeiss Makro-Planar T* 100mm f/2 ZE"                    }, // 12
        {  33, "Carl Zeiss Apo-Sonnar T* 135mm f/2 ZE"                      }, // 13
        {  35, "Canon EF 35-80mm f/4-5.6"                                   },
        {  36, "Canon EF 38-76mm f/4.5-5.6"                                 },
        {  37, "Canon EF 35-80mm f/4-5.6"                                   }, // 0
        {  37, "Tamron 70-200mm f/2.8 Di LD IF Macro"                       }, // 1
        {  37, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro Model A20" }, // 2
        {  37, "Tamron SP AF 17-50mm f/2.8 XR Di II VC LD Aspherical [IF]" }, // 3
        {  37, "Tamron AF 18-270mm f/3.5-6.3 Di II VC LD Aspherical [IF] Macro" }, // 4
        {  38, "Canon EF 80-200mm f/4.5-5.6"                                },
        {  39, "Canon EF 75-300mm f/4-5.6"                                  },
        {  40, "Canon EF 28-80mm f/3.5-5.6"                                 },
        {  41, "Canon EF 28-90mm f/4-5.6"                                   },
        {  42, "Canon EF 28-200mm f/3.5-5.6"                                }, // 0
        {  42, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro Model A20" }, // 1
        {  43, "Canon EF 28-105mm f/4-5.6"                                  },
        {  44, "Canon EF 90-300mm f/4.5-5.6"                                },
        {  45, "Canon EF-S 18-55mm f/3.5-5.6"                               },
        {  46, "Canon EF 28-90mm f/4-5.6"                                   },
        {  47, "Zeiss Milvus 35mm f/2"                                      }, // 0
        {  47, "Zeiss Milvus 50mm f/2 Makro"                                }, // 1
        {  48, "Canon EF-S 18-55mm f/3.5-5.6 IS"                            },
        {  49, "Canon EF-S 55-250mm f/4-5.6 IS"                             },
        {  50, "Canon EF-S 18-200mm f/3.5-5.6 IS"                           },
        {  51, "Canon EF-S 18-135mm f/3.5-5.6 IS"                           },
        {  52, "Canon EF-S 18-55mm f/3.5-5.6 IS II"                         },
        {  53, "Canon EF-S 18-55mm f/3.5-5.6 III"                           },
        {  54, "Canon EF-S 55-250mm f/4-5.6 IS II"                          },
        {  60, "Irix 11mm f/4"                                              },
        {  82, "Canon TS-E 135mm f/4L Macro"                                },
        {  94, "Canon TS-E 17mm f/4L"                                       },
        {  95, "Canon TS-E 24mm f/3.5L II"                                  },
        { 124, "Canon MP-E 65mm f/2.8 1-5x Macro Photo"                     },
        { 125, "Canon TS-E 24mm f/3.5L"                                     },
        { 126, "Canon TS-E 45mm f/2.8"                                      },
        { 127, "Canon TS-E 90mm f/2.8"                                      },
        { 129, "Canon EF 300mm f/2.8L"                                      },
        { 130, "Canon EF 50mm f/1.0L"                                       },
        { 131, "Canon EF 28-80mm f/2.8-4L"                                  }, // 0
        { 131, "Sigma 8mm f/3.5 EX DG Circular Fisheye"                     }, // 1
        { 131, "Sigma 17-35mm f/2.8-4 EX DG Aspherical HSM"                 }, // 2
        { 131, "Sigma 17-70mm f/2.8-4.5 DC Macro"                           }, // 3
        { 131, "Sigma APO 50-150mm f/2.8 EX DC HSM"                         }, // 4
        { 131, "Sigma APO 120-300mm f/2.8 EX DG HSM"                        }, // 5
        { 131, "Sigma 70-200mm f/2.8 APO EX HSM"                            }, // 6
        { 132, "Canon EF 1200mm f/5.6L"                                     },
        { 134, "Canon EF 600mm f/4L IS"                                     },
        { 135, "Canon EF 200mm f/1.8L"                                      },
        { 136, "Canon EF 300mm f/2.8L"                                      }, // 0
        { 136, "Tamron SP 15-30mm f/2.8 Di VC USD A012"                     }, // 1
        { 137, "Canon EF 85mm f/1.2L"                                       }, // 0
        { 137, "Sigma 18-50mm f/2.8-4.5 DC OS HSM"                          }, // 1
        { 137, "Sigma 50-200mm f/4-5.6 DC OS HSM"                           }, // 2
        { 137, "Sigma 18-250mm f/3.5-6.3 DC OS HSM"                         }, // 3
        { 137, "Sigma 24-70mm f/2.8 IF EX DG HSM"                           }, // 4
        { 137, "Sigma 18-125mm f/3.8-5.6 DC OS HSM"                         }, // 5
        { 137, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM | C"                  }, // 6
        { 137, "Sigma 17-50mm f/2.8 OS HSM"                                 }, // 7
        { 137, "Sigma 18-200mm f/3.5-6.3 DC OS HSM [II]"                    }, // 8
        { 137, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"                  }, // 9
        { 137, "Sigma 8-16mm f/4.5-5.6 DC HSM"                              }, // 10
        { 137, "Tamron SP 60mm f/2 Macro Di II"                             }, // 11
        { 137, "Sigma 10-20mm f/3.5 EX DC HSM"                              }, // 12
        { 137, "Sigma 18-35mm f/1.8 DC HSM | A"                             }, // 13
        { 137, "Sigma 12-24mm f/4.5-5.6 DG HSM II"                          }, // 14
        { 138, "Canon EF 28-80mm f/2.8-4L"                                  },
        { 139, "Canon EF 400mm f/2.8L"                                      },
        { 140, "Canon EF 500mm f/4.5L"                                      },
        { 141, "Canon EF 500mm f/4.5L"                                      },
        { 142, "Canon EF 300mm f/2.8L IS"                                   },
        { 143, "Canon EF 500mm f/4L IS"                                     }, // 0
        { 143, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM"                      }, // 1
        { 143, "Sigma 24-105mm f/4 DG OS HSM | A"                           }, // 2
        { 144, "Canon EF 35-135mm f/4-5.6 USM"                              },
        { 145, "Canon EF 100-300mm f/4.5-5.6 USM"                           },
        { 146, "Canon EF 70-210mm f/3.5-4.5 USM"                            },
        { 147, "Canon EF 35-135mm f/4-5.6 USM"                              },
        { 148, "Canon EF 28-80mm f/3.5-5.6 USM"                             },
        { 149, "Canon EF 100mm f/2 USM"                                     },
        { 150, "Canon EF 14mm f/2.8L"                                       }, // 0
        { 150, "Sigma 20mm EX f/1.8"                                        }, // 1
        { 150, "Sigma 30mm f/1.4 DC HSM"                                    }, // 2
        { 150, "Sigma 24mm f/1.8 DG Macro EX"                               }, // 3
        { 150, "Sigma 28mm f/1.8 DG Macro EX"                               }, // 4
        { 150, "Sigma 18-35mm f/1.8 DC HSM | A"                             }, // 5
        { 151, "Canon EF 200mm f/2.8L"                                      },
        { 152, "Canon EF 300mm f/4L IS"                                     }, // 0
        { 152, "Sigma 12-24mm f/4.5-5.6 EX DG ASPHERICAL HSM"               }, // 1
        { 152, "Sigma 14mm f/2.8 EX Aspherical HSM"                         }, // 2
        { 152, "Sigma 10-20mm f/4-5.6"                                      }, // 3
        { 152, "Sigma 100-300mm f/4"                                        }, // 4
        { 153, "Canon EF 35-350mm f/3.5-5.6L"                               }, // 0
        { 153, "Sigma 50-500mm f/4-6.3 APO HSM EX"                          }, // 1
        { 153, "Tamron AF 28-300mm f/3.5-6.3 XR LD Aspherical [IF] Macro"   }, // 2
        { 153, "Tamron AF 18-200mm f/3.5-6.3 XR Di II LD Aspherical [IF] Macro Model A14" }, // 3
        { 153, "Tamron 18-250mm f/3.5-6.3 Di II LD Aspherical [IF] Macro"   }, // 4
        { 154, "Canon EF 20mm f/2.8 USM"                                    }, // 0
        { 154, "Zeiss Milvus 21mm f/2.8"                                    }, // 1
        { 155, "Canon EF 85mm f/1.8 USM"                                    }, // 0
        { 155, "Sigma 14mm f/1.8 DG HSM | A"                                }, // 1
        { 156, "Canon EF 28-105mm f/3.5-4.5 USM"                            }, // 0
        { 156, "Tamron SP 70-300mm f/4-5.6 Di VC USD"                       }, // 1
        { 156, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"               }, // 2
        { 160, "Canon EF 20-35mm f/3.5-4.5 USM"                             }, // 0
        { 160, "Tamron AF 19-35mm f/3.5-4.5"                                }, // 1
        { 160, "Tokina AT-X 124 AF 12-24mm f/4 DX"                          }, // 2
        { 160, "Tokina AT-X 107 AF DX Fish-eye 10-17mm f/3.5-4.5"           }, // 3
        { 160, "Tokina AT-X 116 PRO DX AF 11-16mm f/2.8"                    }, // 4
        { 160, "Tokina AT-X 11-20 F2.8 PRO DX Aspherical 11-20mm f/2.8"     }, // 5
        { 161, "Canon EF 28-70mm f/2.8L"                                    }, // 0
        { 161, "Sigma 24-70mm EX f/2.8"                                     }, // 1
        { 161, "Sigma 24-60mm f/2.8 EX DG"                                  }, // 2
        { 161, "Tamron AF 17-50mm f/2.8 Di-II LD Aspherical"                }, // 3
        { 161, "Tamron 90mm f/2.8"                                          }, // 4
        { 161, "Tamron SP AF 17-35mm f/2.8-4 Di LD Aspherical IF"           }, // 5
        { 161, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro"  }, // 6
        { 162, "Canon EF 200mm f/2.8L"                                      },
        { 163, "Canon EF 300mm f/4L"                                        },
        { 164, "Canon EF 400mm f/5.6L"                                      },
        { 165, "Canon EF 70-200mm f/2.8L"                                   },
        { 166, "Canon EF 70-200mm f/2.8L + 1.4x"                            },
        { 167, "Canon EF 70-200mm f/2.8L + 2x"                              },
        { 168, "Canon EF 28mm f/1.8 USM"                                    }, // 0
        { 168, "Sigma 50-100mm f/1.8 DC HSM | A"                            }, // 1
        { 169, "Canon EF 17-35mm f/2.8L"                                    }, // 0
        { 169, "Sigma 18-200mm f/3.5-6.3 DC OS"                             }, // 1
        { 169, "Sigma 15-30mm f/3.5-4.5 EX DG Aspherical"                   }, // 2
        { 169, "Sigma 18-50mm f/2.8 Macro"                                  }, // 3
        { 169, "Sigma 50mm f/1.4 EX DG HSM"                                 }, // 4
        { 169, "Sigma 85mm f/1.4 EX DG HSM"                                 }, // 5
        { 169, "Sigma 30mm f/1.4 EX DC HSM"                                 }, // 6
        { 169, "Sigma 35mm f/1.4 DG HSM"                                    }, // 7
        { 170, "Canon EF 200mm f/2.8L II"                                   },
        { 171, "Canon EF 300mm f/4L"                                        },
        { 172, "Canon EF 400mm f/5.6L"                                      }, // 0
        { 172, "Sigma 150-600mm f/5-6.3 DG OS HSM | S"                      }, // 1
        { 172, "Sigma 150-500mm f/5-6.3 APO DG OS HSM + 1.4x"               }, // 2
        { 173, "Canon EF 180mm Macro f/3.5L"                                }, // 0
        { 173, "Sigma APO Macro 150mm f/3.5 EX DG IF HSM"                   }, // 1
        { 173, "Sigma 150-500mm f/5-6.3 APO DG OS HSM + 2x"                 }, // 2
        { 174, "Canon EF 135mm f/2L"                                        }, // 0
        { 174, "Sigma 70-200mm f/2.8 EX DG APO OS HSM"                      }, // 1
        { 174, "Sigma 50-500mm f/4.5-6.3 APO DG OS HSM"                     }, // 2
        { 174, "Sigma 150-500mm f/5-6.3 APO DG OS HSM"                      }, // 3
        { 174, "Zeiss Milvus 100mm f/2 Makro"                               }, // 4
        { 174, "Sigma 120-300mm f/2.8 EX APO DG OS HSM"                     }, // 5
        { 175, "Canon EF 400mm f/2.8L"                                      },
        { 176, "Canon EF 24-85mm f/3.5-4.5 USM"                             },
        { 177, "Canon EF 300mm f/4L IS"                                     },
        { 178, "Canon EF 28-135mm f/3.5-5.6 IS"                             },
        { 179, "Canon EF 24mm f/1.4L"                                       },
        { 180, "Canon EF 35mm f/1.4L"                                       }, // 0
        { 180, "Sigma 50mm f/1.4 DG HSM | A"                                }, // 1
        { 180, "Sigma 24mm f/1.4 DG HSM | A"                                }, // 2
        { 180, "Sigma 20mm f/1.4 DG HSM | A"                                }, // 3
        { 180, "Zeiss Milvus 85mm f/1.4"                                    }, // 4
        { 180, "Zeiss Otus 28mm f/1.4 ZE"                                   }, // 5
        { 181, "Canon EF 100-400mm f/4.5-5.6L IS + 1.4x"                    }, // 0
        { 181, "Sigma 150-600mm f/5-6.3 DG OS HSM | S + 1.4x"               }, // 1
        { 182, "Canon EF 100-400mm f/4.5-5.6L IS + 2x"                      }, // 0
        { 182, "Sigma 150-600mm f/5-6.3 DG OS HSM | S + 2x"                 }, // 1
        { 183, "Canon EF 100-400mm f/4.5-5.6L IS"                           }, // 0
        { 183, "Sigma 150mm f/2.8 EX DG OS HSM APO Macro"                   }, // 1
        { 183, "Sigma 105mm f/2.8 EX DG OS HSM Macro"                       }, // 2
        { 183, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"                   }, // 3
        { 183, "Sigma 150-600mm f/5-6.3 DG OS HSM | C"                      }, // 4
        { 184, "Canon EF 400mm f/2.8L + 2x"                                 },
        { 185, "Canon EF 600mm f/4L IS"                                     },
        { 186, "Canon EF 70-200mm f/4L"                                     },
        { 187, "Canon EF 70-200mm f/4L + 1.4x"                              },
        { 188, "Canon EF 70-200mm f/4L + 2x"                                },
        { 189, "Canon EF 70-200mm f/4L + 2.8x"                              },
        { 190, "Canon EF 100mm f/2.8 Macro USM"                             },
        { 191, "Canon EF 400mm f/4 DO IS"                                   },
        { 193, "Canon EF 35-80mm f/4-5.6 USM"                               },
        { 194, "Canon EF 80-200mm f/4.5-5.6 USM"                            },
        { 195, "Canon EF 35-105mm f/4.5-5.6 USM"                            },
        { 196, "Canon EF 75-300mm f/4-5.6 USM"                              },
        { 197, "Canon EF 75-300mm f/4-5.6 IS USM"                           }, // 0
        { 197, "Sigma 18-300mm f/3.5-6.3 DC Macro HSM"                      }, // 1
        { 198, "Canon EF 50mm f/1.4 USM"                                    }, // 0
        { 198, "Zeiss Otus 55mm f/1.4 ZE"                                   }, // 1
        { 198, "Zeiss Otus 85mm f/1.4 ZE"                                   }, // 2
        { 199, "Canon EF 28-80mm f/3.5-5.6 USM"                             },
        { 200, "Canon EF 75-300mm f/4-5.6 USM"                              },
        { 201, "Canon EF 28-80mm f/3.5-5.6 USM"                             },
        { 202, "Canon EF 28-80mm f/3.5-5.6 USM IV"                          },
        { 208, "Canon EF 22-55mm f/4-5.6 USM"                               },
        { 209, "Canon EF 55-200mm f/4.5-5.6"                                },
        { 210, "Canon EF 28-90mm f/4-5.6 USM"                               },
        { 211, "Canon EF 28-200mm f/3.5-5.6 USM"                            },
        { 212, "Canon EF 28-105mm f/4-5.6 USM"                              },
        { 213, "Canon EF 90-300mm f/4.5-5.6 USM"                            }, // 0
        { 213, "Tamron SP 150-600mm f/5-6.3 Di VC USD"                      }, // 1
        { 213, "Tamron 16-300mm f/3.5-6.3 Di II VC PZD Macro"               }, // 2
        { 213, "Tamron SP 35mm f/1.8 Di VC USD"                             }, // 3
        { 213, "Tamron SP 45mm f/1.8 Di VC USD"                             }, // 4
        { 213, "Tamron SP 70-300mm f/4-5.6 Di VC USD"                       }, // 5
        { 214, "Canon EF-S 18-55mm f/3.5-5.6 USM"                           },
        { 215, "Canon EF 55-200mm f/4.5-5.6 II USM"                         },
        { 217, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"                  },
        { 224, "Canon EF 70-200mm f/2.8L IS"                                },
        { 225, "Canon EF 70-200mm f/2.8L IS + 1.4x"                         },
        { 226, "Canon EF 70-200mm f/2.8L IS + 2x"                           },
        { 227, "Canon EF 70-200mm f/2.8L IS + 2.8x"                         },
        { 228, "Canon EF 28-105mm f/3.5-4.5 USM"                            },
        { 229, "Canon EF 16-35mm f/2.8L"                                    },
        { 230, "Canon EF 24-70mm f/2.8L"                                    },
        { 231, "Canon EF 17-40mm f/4L"                                      },
        { 232, "Canon EF 70-300mm f/4.5-5.6 DO IS USM"                      },
        { 233, "Canon EF 28-300mm f/3.5-5.6L IS"                            },
        { 234, "Canon EF-S 17-85mm f/4-5.6 IS USM"                          }, // 0
        { 234, "Tokina AT-X 12-28 PRO DX 12-28mm f/4"                       }, // 1
        { 235, "Canon EF-S 10-22mm f/3.5-4.5 USM"                           },
        { 236, "Canon EF-S 60mm f/2.8 Macro USM"                            },
        { 237, "Canon EF 24-105mm f/4L IS"                                  },
        { 238, "Canon EF 70-300mm f/4-5.6 IS USM"                           },
        { 239, "Canon EF 85mm f/1.2L II"                                    },
        { 240, "Canon EF-S 17-55mm f/2.8 IS USM"                            },
        { 241, "Canon EF 50mm f/1.2L"                                       },
        { 242, "Canon EF 70-200mm f/4L IS"                                  },
        { 243, "Canon EF 70-200mm f/4L IS + 1.4x"                           },
        { 244, "Canon EF 70-200mm f/4L IS + 2x"                             },
        { 245, "Canon EF 70-200mm f/4L IS + 2.8x"                           },
        { 246, "Canon EF 16-35mm f/2.8L II"                                 },
        { 247, "Canon EF 14mm f/2.8L II USM"                                },
        { 248, "Canon EF 200mm f/2L IS"                                     }, // 0
        { 248, "Sigma 24-35mm f/2 DG HSM | A"                               }, // 1
        { 249, "Canon EF 800mm f/5.6L IS"                                   },
        { 250, "Canon EF 24mm f/1.4L II"                                    }, // 0
        { 250, "Sigma 20mm f/1.4 DG HSM | A"                                }, // 1
        { 251, "Canon EF 70-200mm f/2.8L IS II USM"                         },
        { 252, "Canon EF 70-200mm f/2.8L IS II USM + 1.4x"                  },
        { 253, "Canon EF 70-200mm f/2.8L IS II USM + 2x"                    },
        { 254, "Canon EF 100mm f/2.8L Macro IS USM"                         }, // 0
        { 254, "Tamron SP 90mm f/2.8 Di VC USD Macro 1:1"                   }, // 1
        { 255, "Sigma 24-105mm f/4 DG OS HSM | A"                           }, // 0
        { 255, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"                   }, // 1
        { 368, "Sigma 18-35mm f/1.8 DC HSM | A"                             },
        { 488, "Canon EF-S 15-85mm f/3.5-5.6 IS USM"                        },
        { 489, "Canon EF 70-300mm f/4-5.6L IS USM"                          },
        { 490, "Canon EF 8-15mm f/4L Fisheye USM"                           },
        { 491, "Canon EF 300mm f/2.8L IS II USM"                            }, // 0
        { 491, "Tamron SP 24-70mm f/2.8 Di VC USD G2"                       }, // 1
        { 492, "Canon EF 400mm f/2.8L IS II USM"                            },
        { 493, "Canon EF 500mm f/4L IS II USM"                              }, // 0
        { 493, "Canon EF 24-105mm f/4L IS USM"                              }, // 1
        { 494, "Canon EF 600mm f/4L IS II USM"                              },
        { 495, "Canon EF 24-70mm f/2.8L II USM"                             },
        { 496, "Canon EF 200-400mm f/4L IS USM"                             },
        { 499, "Canon EF 200-400mm f/4L IS USM + 1.4x"                      },
        { 502, "Canon EF 28mm f/2.8 IS USM"                                 },
        { 503, "Canon EF 24mm f/2.8 IS USM"                                 },
        { 504, "Canon EF 24-70mm f/4L IS USM"                               },
        { 505, "Canon EF 35mm f/2 IS USM"                                   },
        { 506, "Canon EF 400mm f/4 DO IS II USM"                            },
        { 507, "Canon EF 16-35mm f/4L IS USM"                               },
        { 508, "Canon EF 11-24mm f/4L USM"                                  },
        { 624, "Sigma 14mm f/1.8 DG HSM | A"                                }, // 0
        { 624, "Sigma 150-600mm f/5-6.3 DG OS HSM | C"                      }, // 1
        { 624, "Sigma 150-600mm f/5-6.3 DG OS HSM | C + 1.4x"               }, // 2
        { 747, "Canon EF 100-400mm f/4.5-5.6L IS II USM"                    }, // 0
        { 747, "Tamron SP 150-600mm f/5-6.3 Di VC USD G2"                   }, // 1
        { 748, "Canon EF 100-400mm f/4.5-5.6L IS II USM + 1.4x"             },
        { 750, "Canon EF 35mm f/1.4L II USM"                                },
        { 751, "Canon EF 16-35mm f/2.8L III USM"                            },
        { 752, "Canon EF 24-105mm f/4L IS II USM"                           },
        { 4142,"Canon EF-S 18-135mm f/3.5-5.6 IS STM"                       },
        { 4143,"Canon EF-M 18-55mm f/3.5-5.6 IS STM"                        }, // 0
        { 4143,"Tamron 18-200mm f/3.5-6.3 Di III VC"                        }, // 1
        { 4144,"Canon EF 40mm f/2.8 STM"                                    },
        { 4145,"Canon EF-M 22mm f/2 STM"                                    },
        { 4146,"Canon EF-S 18-55mm f/3.5-5.6 IS STM"                        },
        { 4147,"Canon EF-M 11-22mm f/4-5.6 IS STM"                          },
        { 4148,"Canon EF-S 55-250mm f/4-5.6 IS STM"                         },
        { 4149,"Canon EF-M 55-200mm f/4.5-6.3 IS STM"                       },
        { 4150,"Canon EF-S 10-18mm f/4.5-5.6 IS STM"                        },
        { 4152,"Canon EF 24-105mm f/3.5-5.6 IS STM"                         },
        { 4153,"Canon EF-M 15-45mm f/3.5-6.3 IS STM"                        },
        { 4154,"Canon EF-S 24mm f/2.8 STM"                                  },
        { 4155,"Canon EF-M 28mm f/3.5 Macro IS STM"                         },
        { 4156,"Canon EF 50mm f/1.8 STM"                                    },
        { 4157,"Canon EF-M 18-150mm f/3.5-6.3 IS STM"                       },
        { 4158,"Canon EF-S 18-55mm f/4-5.6 IS STM"                          },
        { 4160,"Canon EF-S 35mm f/2.8 Macro IS STM"                         },
        {36910,"Canon EF 70-300mm f/4-5.6 IS II USM"                        },
        {36912,"Canon EF-S 18-135mm f/3.5-5.6 IS USM"                       },
'''

apertures = '1.0', '1.1', '1.2', '1.4', '1.6', '1.8', '2', '2.2', '2.5', '2.8', '3.2', '3.5', '4', '4.5', '5', '5.6', '6.3', '7.1', '8', '9', '10', '11', '13', '14', '16', '18', '20', '22', '25', '29', '32', '36', '40', '45'
fractions = {0: 0, 1: 12, 2: 20}
aperture_map = {value: (index // 3) * 32 + fractions[index % 3] for index, value in enumerate(apertures)}
aperture_map['3.8'] = '32000' # aperture value cannot be represented in metadata, ignore it by using invalid value

for lens_match in re.finditer('(?P<lens_id>[0-9]+),.*"(?P<lens_description>.*)"', lenses):
    lens = lens_match.group('lens_description')

    metadata_match = re.search('((?P<focal_length_min>[0-9]+)-)?(?P<focal_length_max>[0-9]+)mm.*f/(?P<aperture>[0-9]+(\\.[0-9]+)?)[^+]*(\\+ (?P<tc>[0-9.]+)x)?', lens)
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
