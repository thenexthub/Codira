//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 14, 2026
//!
//! Functionality:
//! - Groth16 proofs over BN254 for an [`R1cs`], through arkworks: setup, prove,
//!   verify, and the byte encodings of keys and proofs.
//!
//! The system is replayed into arkworks' `ConstraintSystem` variable for
//! variable and constraint for constraint, so what is proven is exactly the
//! system `r1cs::lower` built (and the `.r1cs` file describes): the public
//! input is the revealed output, every input is a private witness.
//!
//! Setup is circuit-specific and done by one party here (`codira attest
//! build`): whoever runs it knows the toxic waste and could forge proofs for
//! that circuit. That is fine for development and for a prover who is also
//! the verifier's trusted party; a deployment that needs more runs a
//! multi-party ceremony (e.g. snarkjs over the emitted `.r1cs`) instead.

use ark_bn254::{Bn254, Fr};
use ark_groth16::Groth16;
use ark_relations::r1cs::{
    ConstraintSynthesizer, ConstraintSystemRef, LinearCombination, SynthesisError, Variable,
};
use ark_serialize::{CanonicalDeserialize, CanonicalSerialize};
use ark_snark::SNARK;
use ark_std::rand::{rngs::StdRng, SeedableRng};

use crate::r1cs::{Lc, R1cs, ONE};

pub type ProvingKey = ark_groth16::ProvingKey<Bn254>;
pub type VerifyingKey = ark_groth16::VerifyingKey<Bn254>;
pub type Proof = ark_groth16::Proof<Bn254>;

/// An [`R1cs`] (and, when proving, its witness) as an arkworks circuit.
struct Replay<'a> {
    r1cs: &'a R1cs,
    witness: Option<&'a [Fr]>,
}

impl ConstraintSynthesizer<Fr> for Replay<'_> {
    fn generate_constraints(self, cs: ConstraintSystemRef<Fr>) -> Result<(), SynthesisError> {
        let value = |v: u32| -> Result<Fr, SynthesisError> {
            self.witness
                .and_then(|w| w.get(v as usize).copied())
                .ok_or(SynthesisError::AssignmentMissing)
        };
        let mut vars = Vec::with_capacity(self.r1cs.num_vars as usize);
        vars.push(Variable::One);
        for v in 1..self.r1cs.num_vars {
            vars.push(if v == self.r1cs.output_var {
                cs.new_input_variable(|| value(v))?
            } else {
                cs.new_witness_variable(|| value(v))?
            });
        }
        let lc = |l: &Lc| {
            LinearCombination(
                l.terms()
                    .iter()
                    .map(|(v, c)| {
                        (
                            *c,
                            if *v == ONE {
                                Variable::One
                            } else {
                                vars[*v as usize]
                            },
                        )
                    })
                    .collect(),
            )
        };
        for k in &self.r1cs.constraints {
            cs.enforce_constraint(lc(&k.a), lc(&k.b), lc(&k.c))?;
        }
        Ok(())
    }
}

/// An error from setup, proving or decoding.
#[derive(Debug, thiserror::Error)]
pub enum Groth16Error {
    #[error("Groth16: {0}")]
    Synthesis(#[from] SynthesisError),
    #[error("could not decode: {0}")]
    Decode(#[from] ark_serialize::SerializationError),
    #[error("the operating system's random number generator failed: {0}")]
    Random(String),
}

/// A generator seeded from the operating system.
pub fn os_rng() -> Result<StdRng, Groth16Error> {
    let mut seed = [0u8; 32];
    getrandom::getrandom(&mut seed).map_err(|e| Groth16Error::Random(e.to_string()))?;
    Ok(StdRng::from_seed(seed))
}

/// A deterministic generator, for tests: the keys it gives are insecure.
pub fn seeded_rng(seed: u64) -> StdRng {
    StdRng::seed_from_u64(seed)
}

/// Circuit-specific setup: the proving and verifying keys for `r1cs`.
pub fn setup(r1cs: &R1cs, rng: &mut StdRng) -> Result<(ProvingKey, VerifyingKey), Groth16Error> {
    Ok(Groth16::<Bn254>::circuit_specific_setup(
        Replay {
            r1cs,
            witness: None,
        },
        rng,
    )?)
}

/// A proof that `witness` satisfies `r1cs` (the witness must come from
/// [`R1cs::witness`]).
pub fn prove(
    r1cs: &R1cs,
    pk: &ProvingKey,
    witness: &[Fr],
    rng: &mut StdRng,
) -> Result<Proof, Groth16Error> {
    Ok(Groth16::<Bn254>::prove(
        pk,
        Replay {
            r1cs,
            witness: Some(witness),
        },
        rng,
    )?)
}

/// Whether `proof` shows that some private inputs make the circuit output
/// `public` (the revealed value, as [`R1cs::public_inputs`] gives it).
pub fn verify(vk: &VerifyingKey, public: &[Fr], proof: &Proof) -> Result<bool, Groth16Error> {
    let pvk = Groth16::<Bn254>::process_vk(vk)?;
    Ok(Groth16::<Bn254>::verify_with_processed_vk(
        &pvk, public, proof,
    )?)
}

/// Compressed canonical bytes of a key or proof.
pub fn to_bytes(value: &impl CanonicalSerialize) -> Vec<u8> {
    let mut bytes = Vec::new();
    value
        .serialize_compressed(&mut bytes)
        .expect("serializing to a Vec cannot fail");
    bytes
}

/// Decodes (and validates: points on the curve, in the right subgroup) a key
/// or proof written by [`to_bytes`].
pub fn from_bytes<T: CanonicalDeserialize>(bytes: &[u8]) -> Result<T, Groth16Error> {
    Ok(T::deserialize_compressed(bytes)?)
}
