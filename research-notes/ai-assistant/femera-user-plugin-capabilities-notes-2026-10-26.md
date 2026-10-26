# Femera user extensions and plugin capabilities

Date: 2026-10-26

Status: architectural notes; no implementation changes made.

Companion proposal:
[Runtime interfaces and CRTP migration plan](./femera-runtime-interfaces-plan-2026-10-26.md).

## Objective

Make user-defined scientific implementations a first-class architectural
requirement. Scientists, engineers, and other API users should be able to add
features without editing Femera's core or introducing dependencies on optional
backend headers.

Preserve the existing intent of the identifiers declared in
[type.hpp](../../src/femera/type.hpp):

- `Task_type`: built-in implementations.
- `User_type`: application-defined implementations known at build time.
- `Plug_type`: runtime plugin implementations.
- `Work_type`: the common identifier used by internal machinery.

Distinguish compile-time enumeration from runtime registration. They serve
related but different extension models.

## 1. Separate identity, family, and capability

| Property | Question | Example |
|----------|----------|---------|
| Implementation identifier | What implementation is this? | A user's constitutive model |
| Family | What category of object is it? | `Phys` |
| Capability/interface | What can callers do with it? | Evaluate constitutive response |

An integer identifier can select an implementation, but does not establish C++
inheritance or provide a callable interface.

A user-defined scientific implementation should satisfy the appropriate public
family or capability contract. Conceptually:

```cpp
class MyMaterial final : public femera::Phys<MyMaterial> {
    // Implement the documented material hooks.
};
```

In the companion proposal, that CRTP adapter would derive from a real
non-template family interface. Femera could operate on that interface without
knowing `MyMaterial`.

Apply this design not only to `Proc`, `Data`, `Task`, and `Test`, but also to
physics, grid, and algorithm families where scientific extensions are
particularly important.

## 2. Keep enum extension for coordinated application builds

Allocating application identifiers after an internal boundary is reasonable when
one application controls allocation.

Using the current boundary names, an illustrative definition is:

```cpp
namespace femera {
enum class User_type : fmr::Enum_int {
    MyMaterial = fmr::Enum_int(Task_type::User) + 1,
    MySolver,
    Plug
};
}
```

The existing [task_cast overloads](../../src/femera/type.ipp) allow these
identifiers to become `Work_type`.

Two constraints must be explicit:

1. There must be one consistent definition of `User_type` across the program.
   Independently authored libraries cannot each define an incompatible
   `femera::User_type`.
2. Automatically assigned numbers are not inherently stable across Femera
   versions. Inserting a built-in before the boundary shifts subsequent values.

This mechanism is suitable for a coordinated application build, but should not
be the sole identity mechanism for independently distributed plugins or
persisted datasets.

Keep optional built-in enumerators present regardless of feature availability,
as the current declarations do. Enabling CGNS or MPI should not renumber the
remaining identifiers.

### Define boundary semantics precisely

Either convention can work:

- `end` is the first unused value; extension identifiers start at `end`.
- `last` is the last reserved value; extension identifiers start after `last`.

Choose and document one convention consistently. Sentinel values should not
accidentally become implementation identifiers.

## 3. Runtime plugins need registration, not enum extension

A fixed-underlying-type opaque enum declaration such as:

```cpp
enum class Plug_type : fmr::Enum_int;
```

can represent integer values without listing enumerators. Loading a plugin,
however, cannot add named enumerators to an already compiled C++ program.

Independent plugins also cannot safely each choose the first integer after an
application boundary: their identifiers will collide.

Use:

```text
Stable external key
    -> host registry
        -> process-local Work_type
```

Illustrative stable keys:

```text
org.example.materials.viscoelastic
org.example.solvers.custom-newton
```

The host allocates a unique numeric identifier at registration and returns it
to the plugin. Plugins do not independently choose runtime numeric IDs.

This preserves fast integer lookup while supporting independently authored
extensions.

Persist the stable key, not the runtime-assigned integer, in configurations,
checkpoints, and other data expected to outlive the process. Persist relevant
implementation/schema version information where reproducibility requires it.

`Plug_type` can remain a compatibility wrapper, but its role would be
"host-assigned plugin identifier," not an enum extended by runtime modules.

## 4. Register implementation factories

A registry connects implementation identity to construction.

A conceptual registration record contains:

```text
Stable implementation key
Display name
Family
Factory
Required interface/API version
Implementation version
Owning plugin/module, if dynamically loaded
```

A factory constructs an object satisfying the requested family interface and
returns ownership through Femera's supported ownership mechanism.

Validate registration and construction explicitly:

- Duplicate keys.
- Invalid or exhausted numeric ranges.
- Unsupported API/interface versions.
- Missing dependencies.
- Factory failure.
- Incorrect family membership.

A duplicate key should fail unless replacement is a deliberately supported
operation. Do not silently select an implementation based on load order.

### Registration is distinct from instantiation

One registered implementation can have multiple instances:

```text
Implementation: org.example.materials.viscoelastic
Instances: material A, material B, material C
```

Instances share implementation identity but have distinct parameters and state.
The existing indexed lookup concept can continue to distinguish multiple
objects of the same implementation type.

## 5. Make normal extension require no core edits

The intended researcher workflow is:

1. Include a documented extension header.
2. Derive from the appropriate adapter.
3. Implement the required hooks.
4. Register a factory.
5. Select the implementation through configuration or an API call.

Users should not need to:

- Edit Femera's built-in enums.
- Add cases to core dispatch switches.
- Expose concrete extension classes in public Femera headers.
- Rebuild unrelated backends.
- Cast their objects to unrelated built-in facade types.

For build-time extensions, use an explicit registration function called by the
application. Prefer this to global-constructor registration, which introduces
initialization-order and static-library linking surprises.

A dynamically loaded plugin can expose an entry point that performs equivalent
registration through a host-provided API.

The public facades remain application entry points. Registered implementations
of an existing operation must be callable through their real interfaces, rather
than requiring a new facade switch case for every implementation.

## 6. Distinguish new implementations from new interfaces

### Implementing an existing scientific contract

Examples:

- Another solver.
- Another material model.
- Another file format.
- Another test.

These should require implementing an established interface and registering a
factory, not modifying the core.

### Introducing an entirely new capability

Examples:

- A new sensitivity-analysis API.
- A specialized coupling protocol.
- A novel data service outside existing contracts.

A new integer identifier cannot make core Femera understand new methods.

Choose between:

- An application-owned interface used by code that knows that interface.
- A versioned capability-query mechanism when independently developed
  components must discover and share additional interfaces.

Do not introduce a universal capability system without concrete requirements.
First make implementations of existing scientific contracts straightforward.
Add capability discovery when actual extension use cases justify it.

## 7. Choose the runtime plugin ABI explicitly

| Model | Benefits | Constraints |
|-------|----------|-------------|
| C++ plugins built against a compatible Femera toolchain | Natural inheritance, templates, existing objects | Compiler, standard library, build settings, and interface compatibility requirements |
| Versioned C ABI with opaque handles/function tables | More controlled binary boundary; easier integration with other languages | More adapter code and explicit ownership/error conventions |

For plugins normally rebuilt alongside a scientific application, a documented
compatible-build C++ model is a reasonable starting point.

For independently distributed binaries or cross-language plugins, consider a
versioned C boundary. A C-named entry point alone does not make a C++ object
interface ABI-stable.

Whichever model is chosen:

- Keep a module loaded while any of its objects, factories, callbacks, or worker
  activities remain live.
- Define which side destroys objects and releases allocations.
- Translate failures at the plugin boundary.
- Do not let incompatible exceptions or ownership conventions cross the
  boundary.
- Treat in-process plugins as trusted native code, not sandboxed extensions.

## 8. Integrate with the runtime-interface migration

Add these requirements to the companion architectural plan:

1. Public non-template scientific interfaces, with CRTP adapters for implementing
   them.
2. `Work_type` as a common runtime identifier, retaining enum conversion
   convenience.
3. Coordinated `User_type` allocation for application builds.
4. Stable external keys and host-assigned IDs for independently authored
   runtime plugins.
5. A factory registry shared by built-in, application-defined, and dynamically
   loaded implementations.
6. No mandatory core switch edits for a new implementation of an existing
   contract.
7. Explicit version, ownership, error, and module-lifetime policies.

Built-ins and user implementations should enter through the same interfaces
and registration path. Built-ins can be registered explicitly at startup.
Dynamic loading is another source of registrations, not a separate object
architecture.

## Validation considerations

- Register and instantiate a user implementation without editing core enums or
  dispatch switches.
- Build an application extension without unrelated optional backend headers.
- Register two independent plugins without numeric identifier collisions.
- Reject duplicate stable keys and unsupported interface versions explicitly.
- Verify correct dispatch through family interfaces.
- Verify multiple instances retain separate parameters and state.
- Resolve persisted stable keys correctly despite registration-order changes.
- Report unavailable implementations and factory failures explicitly.
- Verify module lifetime extends beyond all plugin-owned objects and callbacks.
- Exercise the documented supported toolchain/ABI combination.

## Decisions before implementation

- Stable-key naming and ownership convention.
- Supported scope of coordinated `User_type` definitions.
- Numeric allocation policy and handling of boundary/range exhaustion.
- Required scientific interfaces and extension hooks.
- Supported plugin ABI and compatibility/version policy.
- Whether runtime loading and unloading are both required.
- Factory construction parameters and error reporting.
- Whether concrete use cases require additional capability discovery.

## Recommendation

Preserve the distinction between built-in, build-time user, and runtime plugin
implementations. Use real interfaces for behavior, stable keys for external
identity, and host-assigned numeric IDs for runtime selection. Make built-in
and user implementations follow the same registration and dispatch contracts.
