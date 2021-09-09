/** 
 * HELPER definitions for TCG code generation.
 */
#if defined(TCG_GEN) && defined(CONFIG_QFLEX)
DEF_HELPER_2(qflex_magic_inst, void, env, i64)

#elif !defined(CONFIG_QFLEX) 
// Empty definitions when disabled
void HELPER(qflex_magic_inst)(CPUARMState *env, uint64_t nop_op);

#endif
