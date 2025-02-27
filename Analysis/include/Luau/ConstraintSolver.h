// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details

#pragma once

#include "Luau/Error.h"
#include "Luau/Variant.h"
#include "Luau/ConstraintGraphBuilder.h"
#include "Luau/TypeVar.h"

#include <vector>

namespace Luau
{

// TypeId, TypePackId, or Constraint*. It is impossible to know which, but we
// never dereference this pointer.
using BlockedConstraintId = const void*;

struct ConstraintSolver
{
    TypeArena* arena;
    InternalErrorReporter iceReporter;
    // The entire set of constraints that the solver is trying to resolve.
    std::vector<const Constraint*> constraints;
    Scope2* rootScope;
    std::vector<TypeError> errors;

    // This includes every constraint that has not been fully solved.
    // A constraint can be both blocked and unsolved, for instance.
    std::unordered_set<const Constraint*> unsolvedConstraints;

    // A mapping of constraint pointer to how many things the constraint is
    // blocked on. Can be empty or 0 for constraints that are not blocked on
    // anything.
    std::unordered_map<const Constraint*, size_t> blockedConstraints;
    // A mapping of type/pack pointers to the constraints they block.
    std::unordered_map<BlockedConstraintId, std::vector<const Constraint*>> blocked;

    explicit ConstraintSolver(TypeArena* arena, Scope2* rootScope);

    /**
     * Attempts to dispatch all pending constraints and reach a type solution
     * that satisfies all of the constraints, recording any errors that are
     * encountered.
     **/
    void run();

    bool done();

    bool tryDispatch(const Constraint* c);
    bool tryDispatch(const SubtypeConstraint& c);
    bool tryDispatch(const PackSubtypeConstraint& c);
    bool tryDispatch(const GeneralizationConstraint& c);
    bool tryDispatch(const InstantiationConstraint& c, const Constraint* constraint);

    /**
     * Marks a constraint as being blocked on a type or type pack. The constraint
     * solver will not attempt to dispatch blocked constraints until their
     * dependencies have made progress.
     * @param target the type or type pack pointer that the constraint is blocked on.
     * @param constraint the constraint to block.
     **/
    void block_(BlockedConstraintId target, const Constraint* constraint);
    void block(const Constraint* target, const Constraint* constraint);
    void block(TypeId target, const Constraint* constraint);
    void block(TypePackId target, const Constraint* constraint);

    /**
     * Informs the solver that progress has been made on a type or type pack. The
     * solver will wake up all constraints that are blocked on the type or type pack,
     * and will resume attempting to dispatch them.
     * @param progressed the type or type pack pointer that has progressed.
     **/
    void unblock_(BlockedConstraintId progressed);
    void unblock(const Constraint* progressed);
    void unblock(TypeId progressed);
    void unblock(TypePackId progressed);

    /**
     * Returns whether the constraint is blocked on anything.
     * @param constraint the constraint to check.
     */
    bool isBlocked(const Constraint* constraint);

    void reportErrors(const std::vector<TypeError>& errors);

    /**
     * Creates a new Unifier and performs a single unification operation. Commits
     * the result and reports errors if necessary.
     * @param subType the sub-type to unify.
     * @param superType the super-type to unify.
     */
    void unify(TypeId subType, TypeId superType);

    /**
     * Creates a new Unifier and performs a single unification operation. Commits
     * the result and reports errors if necessary.
     * @param subPack the sub-type pack to unify.
     * @param superPack the super-type pack to unify.
     */
    void unify(TypePackId subPack, TypePackId superPack);
};

void dump(Scope2* rootScope, struct ToStringOptions& opts);

} // namespace Luau
