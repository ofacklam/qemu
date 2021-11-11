devteroflex_ss.add([files(
   'devteroflex.pb-c.c',
   'file-ops.c',
   'page-demander.c',
   'verification.c',
   'custom-instrumentation.c',
   'tracing.c',
   'devteroflex.c',
 ), devteroflex])

if 'AWS_FPGA' in config_host
  aws_ss = ss.source_set()
  devteroflex_ss.add_all(aws_ss) 
  subdir('aws')
else
  simulation_ss = ss.source_set()
  subdir('simulation')
  devteroflex_ss.add_all(simulation_ss) 
endif
