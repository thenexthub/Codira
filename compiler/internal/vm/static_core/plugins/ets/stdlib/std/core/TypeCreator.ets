/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package std.core;

class AllowedAttributes {
    public static readonly FIELD = Attributes.STATIC | Attributes.READONLY
    public static readonly METHOD = Attributes.STATIC | Attributes.FINAL | Attributes.ABSTRACT | Attributes.CONSTRUCTOR | Attributes.NATIVE | Attributes.ASYNC | Attributes.GETTER | Attributes.SETTER
    public static readonly LAMBDA = Attributes.ASYNC
    public static readonly CLASS = Attributes.FINAL
    public static readonly PARAMETER = Attributes.REST | Attributes.OPTIONAL
}

/**
 * Error that is thrown when exception in create-api occurs
 * @note both messages and causes may be different on each platforms
 */
export class TypeAPICreateError extends Error {
    public constructor(msg: string) {
        super(msg)
    }
}

/**
 * Type of function that collects all reachable types for creator context
 */
type CollectTypesCB = (tc: TypeOrCreator) => void

/**
 * Base for all type creators, supports creation of {@link Type}
 */
export abstract class TypeCreator {
    protected typ?: Type // not undefined if already created
    protected ctxItemPtr: long = 0 // resides in ctx, no need to free

    /**
     * Forces creation of `this` creator and creators it depends on
     * @returns created Type instance
     */
    public create(): Type {
        if (this.typ == undefined) {
            const ctx = new TypeCreatorCtx()
            const targetLinker = this.getTargetLinker()
            try {
                ctx.create(this, targetLinker)
            } finally {
                ctx.deinit()
            }
        }
        return this.typ!
    }

    protected abstract getTargetLinker(): RuntimeLinker

    /**
     * Traverses all types on which this `TypeCreator` depends
     * @param collector callback to be called with every type
     */
    public abstract collectTypes(collector: CollectTypesCB): void

    /**
     * Must declare up to the point where future type descriptor is determinated
     * @param ctx current context
     */
    public abstract declare(ctx: TypeCreatorCtx): void

    /**
     * Defines rest of the body, called after {@link declare}
     * @param ctx current context
     */
    public abstract define(ctx: TypeCreatorCtx): void

    /** Makes class sealed in stdlib */
    public constructor() {}

    /**
     * Functions that verifies that this can be modified
     */
    protected checkNotCreated(): void {
        if (this.typ != undefined) throw new TypeAPICreateError("type already created, no modifications allowed")
    }

    /**
     * @returns type descriptor that created type will habe
     * @note will be called only after {@link define}
     */
    public getTypeDescriptor(): RuntimeTypeDescriptor {
        if (this.typ != undefined) {
            return this.typ!.getTypeDesc()
        }
        if (this.ctxItemPtr === 0) {
            throw new AssertionError("this.ctxItemPtr is 0")
        }
        return TypeCreatorCtx.getTypeDescFromPointer(this.ctxItemPtr)
    }

    public setTyp(typ_: Type) {
        this.typ = typ_ 
    }

    public getTyp(): Type | undefined {
        return this.typ
    }
}

/**
 * semantically same as Type | TypeCreator, but provides unified way to get descriptor
 */
export abstract class TypeOrCreator {
    /**
     * @returns type descriptor or future type descriptor
     */
    abstract getTypeDescriptor(): RuntimeTypeDescriptor

    /**
     * @returns created {@link TypeOrCreator_Type}
     */
    static from(typ: Type): TypeOrCreator {
        return new TypeOrCreator_Type(typ)
    }

    /**
     * @returns created {@link TypeOrCreator_Creator} or TypeOrCreator_Type if it was already created
     */
    static from(typ: TypeCreator): TypeOrCreator {
        if (typ.getTyp() != undefined) {
            return new TypeOrCreator_Type(typ.getTyp()!)
        }
        return new TypeOrCreator_Creator(typ)
    }
}

final class TypeOrCreator_Type extends TypeOrCreator {
    private typ: Type

    override getTypeDescriptor(): RuntimeTypeDescriptor {
        return this.typ.getTypeDesc()
    }

    constructor(typ: Type) {
        this.typ = typ
    }
}

final class TypeOrCreator_Creator extends TypeOrCreator {
    typ: TypeCreator

    override getTypeDescriptor(): RuntimeTypeDescriptor {
        return this.typ.getTypeDescriptor()
    }

    constructor(typ: TypeCreator) {
        this.typ = typ
    }
}

export final class TypeCreatorCtx {
    public ctxPtr: long
    /** all objects that were passed to cpp code as their ids (indexes in this `Array`) */
    private objects: Array<Object> = new Array<Object>();

    private static native createCtx(): long
    private static native destroyCtx(ctxPtr: long): void

    /**
     * Commits all created types into runtime.
     *
     * @param ctxPtr pointer to native context.
     *
     * @param objects array of saved objects passed to static constructor.
     *
     * @param targetLinker in which context code will be loaded.
     *
     * @returns string in case of occurred errors.
     *
     * @throws errors happened on class initialization.
     */
    private static native commit(ctxPtr: long, objects: FixedArray<Object>, targetLinker: RuntimeLinker): string | undefined

    private static native getError(ctxPtr: long): string | undefined
    public static native getObjectsArrayForCCtor(ctxPtr: long): FixedArray<Object>

    public static native getTypeDescFromPointer(ctxItemPtr: long): RuntimeTypeDescriptor

    public static native classCreate(ctxPtr: long, name: string, attrs: int): long
    public static native classSetBase(classPtr: long, base: RuntimeTypeDescriptor): string | undefined
    public static native classAddIface(classPtr: long, iface: RuntimeTypeDescriptor): string | undefined
    public static native classAddField(classPtr: long, name: string, field: RuntimeTypeDescriptor, attrs: int, access: int): string | undefined

    public static native interfaceCreate(ctxPtr: long, name: string): long
    public static native interfaceAddBase(ifacePtr: long, base: RuntimeTypeDescriptor): string | undefined

    public static native lambdaTypeCreate(ctxPtr: long, attrs: int): long
    public static native lambdaTypeAddParam(ftPtr: long, param: RuntimeTypeDescriptor, attrs: int): string | undefined
    public static native lambdaTypeAddResult(ftPtr: long, result: RuntimeTypeDescriptor): string | undefined
    public static native lambdaTypeAdd(ftPtr: long): string | undefined

    public static native methodCreate(ctxItemPtr: long, name: string, attrs: int): long
    public static native methodAddAccessMod(methPtr: long, access: int): string | undefined
    public static native methodAddParam(methPtr: long, td: RuntimeTypeDescriptor, name: string, attrs: int): string | undefined
    public static native methodAddResult(methPtr: long, result: RuntimeTypeDescriptor): string | undefined
    public static native methodAddBodyFromMethod(methPtr: long, methodType: MethodType): string | undefined
    public static native methodAddBodyFromLambda(methPtr: long, id: int, lambdaType: LambdaType): string | undefined
    public static native methodAddBodyFromErasedLambda(methPtr: long, id: int): string | undefined
    public static native methodAddBodyDefault(methPtr: long): string | undefined
    public static native methodAdd(methPtr: long): string | undefined

    /**
     * checks that context is errorneous
     * used after long-returning types (that can't throw an exception explicitly)
     */
    public checkError(): void {
        this.checkError(TypeCreatorCtx.getError(this.ctxPtr))
    }

    /**
     * Checks for error if pointer is invalid
     * @param ptr pointer to one of items allocated in the context
     * @returns `ptr`
     */
    public checkCtxItemPtr(ptr: long): long {
        if (ptr == 0) {
            this.checkError()
        }
        return ptr
    }

    /**
     * Cleans up offheap memory for this context.
     * Must be called.
     */
    public deinit(): void {
        if (this.ctxPtr === 0) {
            throw new AssertionError("cannot call deinit when ctxPtr is null")
        }
        TypeCreatorCtx.destroyCtx(this.ctxPtr)
        this.ctxPtr = 0
    }

    /** default constructor */
    public constructor() {
        this.ctxPtr = TypeCreatorCtx.createCtx()
    }

    /**
     * Runs entire type creation routine, including all types collection
     * @param first creator with must be created
     * @param targetLinker in which type creation must be committed
     */
    public create(first: TypeCreator, targetLinker: RuntimeLinker): void {
        const all = new Set<TypeCreator>()
        const visited = new Set<TypeCreator>()

        const order = new Array<TypeCreator>()

        // bfs traverser
        const push: CollectTypesCB = (tc: TypeOrCreator): void => {
            if (tc instanceof TypeOrCreator_Type) {
                return
            }
            const typeCreator = (tc as TypeOrCreator_Creator).typ
            if (typeCreator.getTyp() != undefined) {
                return
            }
            if (visited.has(typeCreator)) {
                return
            }
            order.push(typeCreator)
            all.add(typeCreator)
        }

        push(new TypeOrCreator_Creator(first))

        // bfs
        let was = true
        while (was) {
            was = false
            const asArr = new Array<TypeCreator>()

            all.forEach((creator: TypeCreator): void => {
                asArr.push(creator)
            })
            for (let i = 0; i < asArr.length; i++) {
                const el = asArr.$_get(i)
                if (!visited.has(el)) {
                    was = true
                    visited.add(el)
                    el.collectTypes(push)
                }
            }
        }

        // reversing list
        for (let i = 0; i < order.length / 2; i++) {
            const j = order.length - 1 - i
            const tmp = order.$_get(i)
            order.$_set(i, order.$_get(j))
            order.$_set(j, tmp)
        }

        // forward declare all entities
        for (let i = 0; i < order.length; i++) {
            order.$_get(i).declare(this)
        }

        // defines all entities (i.e. provides bodies)
        for (let i = 0; i < order.length; i++) {
            order.$_get(i).define(this)
        }

        this.checkError()

        let objsArray: FixedArray<Object> = new FixedArray<Object>(this.objects.length)
        for (let i = 0; i < this.objects.length; i++) {
            objsArray[i] = this.objects.$_get(i) as Object
        }
        this.checkError(TypeCreatorCtx.commit(this.ctxPtr, objsArray, targetLinker))

        // assignes actual runtime types to all creators
        for (let i = 0; i < order.length; i++) {
            const tc = order.$_get(i)
            let resolved = Type.resolve(tc.getTypeDescriptor(), targetLinker) 
            if (resolved == undefined) {
                throw new TypeAPICreateError("Can not resolve Type")
            }
            tc.setTyp(resolved!)
        }
    }

    /**
     * Saves object that is used in generated code.
     * Example: lambdas of body methods.
     * @returns id of saved object that can be passed to runtime intrinsics
     */
    public registerObject(o: Object): int {
        const r = this.objects.length
        this.objects.push(o)
        return r.toInt()
    }

    /**
     * Checks if ctx function returned an error
     * @throws TypeAPICreateError if error was found
     */
    public checkError(arg: string | undefined): void {
        if (arg != undefined) {
            throw new TypeAPICreateError(arg as string)
        }
    }
}

/**
 * Creator for {@link ClassType}
 */
export final class ClassTypeCreator extends TypeCreator {
    private base: TypeOrCreator = TypeOrCreator.from(ObjectType)
    private name: string
    private ifaces: Array<TypeOrCreator> = new Array<TypeOrCreator>()
    private fields: Array<FieldCreator> = new Array<FieldCreator>()
    private methods: Array<MethodCreator> = new Array<MethodCreator>()
    private attrs: HasAttributes
    private targetLinker: RuntimeLinker

    /**
     * @param name fully qualified name of class type, can include only [A-Za-z0-9_.]
     */
    public constructor(name: string) {
        super()
        this.attrs = new HasAttributes(new FreezableCreator(this), AllowedAttributes.CLASS)
        this.name = name
        this.targetLinker = getNearestNonBootRuntimeLinker()!
    }

    /**
     * Forces creation of `this` creator and creators it depends on
     * @returns created Type instance
     */
    public override create(): ClassType {
        return super.create() as ClassType
    }

    /**
     * Sets base class (super, that in ets follows `extends` keyword)
     * @param base new base
     * @returns this
     */
    public addBaseType(base: ClassType): ClassTypeCreator {
        this.checkNotCreated()
        this.base = TypeOrCreator.from(base)
        return this;
    }

    /**
     * Sets base class (super, that in ets follows `extends` keyword)
     * @param base new base
     * @returns this
     */
    public addBaseType(base: ClassTypeCreator): ClassTypeCreator {
        this.checkNotCreated()
        this.base = TypeOrCreator.from(base)
        return this;
    }

    /**
     * Adds implemented interface (in ets it follows `implements` keyword)
     * @param iface interface to implement
     * @returns this
     */
    public addInterface(iface: InterfaceType): ClassTypeCreator {
        this.ifaces.push(TypeOrCreator.from(iface))
        return this;
    }

    /**
     * Adds implemented interface (in ets it follows `implements` keyword)
     * @param iface interface to implement
     * @returns this
     */
    public addInterface(iface: InterfaceTypeCreator): ClassTypeCreator {
        this.ifaces.push(TypeOrCreator.from(iface))
        return this;
    }

    /**
     * Adds field
     * @param fld field to add
     * @returns this
     */
    public addField(fld: FieldCreator): ClassTypeCreator {
        fld.getFrozen().freeze()
        this.fields.push(fld)
        return this;
    }

    public override collectTypes(collector: CollectTypesCB): void {
        collector(this.base)
        for (let i = 0; i < this.ifaces.length; i++) {
            collector(this.ifaces.$_get(i))
        }
        for (let i = 0; i < this.fields.length; i++) {
            collector(this.fields.$_get(i).typ)
        }
        for (let i = 0; i < this.methods.length; i++) {
            const m = this.methods.$_get(i)
            collector(m.result)
            for (let i = 0; i < m.params.length; i++) {
                collector(m.params.$_get(i).typ)
            }
        }
    }

    public override declare(ctx: TypeCreatorCtx): void {
        if (this.ctxItemPtr !== 0) {
            throw new AssertionError("this.ctxItemPtr is not 0")
        }
        this.ctxItemPtr = ctx.checkCtxItemPtr(TypeCreatorCtx.classCreate(ctx.ctxPtr, this.name, this.attrs.get()))
    }

    public override define(ctx: TypeCreatorCtx): void {
        const classPtr = this.ctxItemPtr
        ctx.checkError(TypeCreatorCtx.classSetBase(classPtr, this.base.getTypeDescriptor()))
        for (let i = 0; i < this.ifaces.length; i++) {
            ctx.checkError(TypeCreatorCtx.classAddIface(classPtr, this.ifaces.$_get(i).getTypeDescriptor()))
        }
        for (let i = 0; i < this.fields.length; i++) {
            const f = this.fields.$_get(i);
            ctx.checkError(TypeCreatorCtx.classAddField(classPtr, f.name, f.typ.getTypeDescriptor(), f.attrs.get(), f.getAccessMod()))
        }
        for (let i = 0; i < this.methods.length; i++) {
            const m = this.methods.$_get(i)
            m.addToCtxItemPtr(ctx, classPtr)
        }
    }

    protected override getTargetLinker(): RuntimeLinker {
        return this.targetLinker
    }

    /**
     * Adds method
     * @param mb method to add
     * @returns this
     * @note prevents future modifications of `mb`
     */
    public addMethod(mb: MethodCreator): ClassTypeCreator {
        mb.frozen.freeze()
        this.methods.push(mb)
        return this;
    }

    /**
     * Sets all provided attributes to `true`
     * @param attrs attribute to set
     * @returns this
     */
    public addAttributes(attrs: int): ClassTypeCreator {
        this.attrs.add(attrs)
        return this;
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addFinal(): ClassTypeCreator {
        this.attrs.add(Attributes.FINAL, true)
        return this;
    }
}

/**
 * Creator for {@link InterfaceType}
 */
export final class InterfaceTypeCreator extends TypeCreator {
    private name: string
    private bases: Array<TypeOrCreator> = new Array<TypeOrCreator>()
    private methods: Array<MethodCreator> = new Array<MethodCreator>()
    private targetLinker: RuntimeLinker

    /**
     * @param name name of interface
     * @see ClassTypeCreator.constructor
     */
    public constructor(name: string) {
        super()
        this.name = name
        this.targetLinker = getNearestNonBootRuntimeLinker()!
    }

    /**
     * Forces creation of `this` creator and creators it depends on
     * @returns created Type instance
     */
    public override create(): InterfaceType {
        return super.create() as InterfaceType
    }

    /**
     * Adds implemented interface (in ets it follows `extends` keyword)
     * @param iface interface to implement
     * @returns this
     */
    public addInterface(base: InterfaceType): InterfaceTypeCreator {
        this.checkNotCreated()
        this.bases.push(TypeOrCreator.from(base))
        return this;
    }

    /**
     * Adds implemented interface (in ets it follows `extends` keyword)
     * @param iface interface to implement
     * @returns this
     */
    public addInterface(base: InterfaceTypeCreator): InterfaceTypeCreator {
        this.checkNotCreated()
        this.bases.push(TypeOrCreator.from(base))
        return this;
    }

    /**
     * Adds getter and optionally setter to `this` interface
     * @param fld property description
     * @returns this
     */
    public addField(fld: FieldCreator): InterfaceTypeCreator {
        const getter = new MethodCreator(fld.name).addResult(fld.typ).addGetter()
        this.addMethod(getter)
        if (!fld.attrs.has(Attributes.READONLY)) {
            const setter = new MethodCreator(fld.name).addParameter(new ParameterCreator(fld.typ)).addSetter()
            this.addMethod(setter)
        }
        return this;
    }

    /**
     * Adds method to `this` interface
     * @param mc method to add
     * @returns this
     */
    public addMethod(mc: MethodCreator): InterfaceTypeCreator {
        if ((mc.attrs.has(Attributes.STATIC))) {
            throw new TypeAPICreateError("interface can't have static method " + mc)
        }
        mc.addAbstract()
        mc.addAccessModifier(AccessModifier.PUBLIC)
        mc.frozen.freeze()
        this.methods.push(mc)
        return this;
    }

    public override collectTypes(collector: CollectTypesCB): void {
        for (let i = 0; i < this.bases.length; i++) {
            collector(this.bases.$_get(i))
        }
        for (let i = 0; i < this.methods.length; i++) {
            const m = this.methods.$_get(i)
            collector(m.result)
            for (let i = 0; i < m.params.length; i++) {
                collector(m.params.$_get(i).typ)
            }
        }
    }

    public override define(ctx: TypeCreatorCtx): void {
        const classPtr = this.ctxItemPtr
        for (let i = 0; i < this.bases.length; i++) {
            ctx.checkError(TypeCreatorCtx.interfaceAddBase(classPtr, this.bases.$_get(i).getTypeDescriptor()))
        }
        for (let i = 0; i < this.methods.length; i++) {
            const m = this.methods.$_get(i)
            m.addToCtxItemPtr(ctx, classPtr)
        }
    }

    public override declare(ctx: TypeCreatorCtx): void {
        if (this.ctxItemPtr !== 0) {
            throw new AssertionError("this.ctxItemPtr is not 0")
        }
        this.ctxItemPtr = ctx.checkCtxItemPtr(TypeCreatorCtx.interfaceCreate(ctx.ctxPtr, this.name))
    }

    protected override getTargetLinker(): RuntimeLinker {
        return this.targetLinker
    }
}

/**
 * Creator for {@link LambdaType}
 */
export final class LambdaTypeCreator extends TypeCreator {
    public params: Array<ParameterCreator> = new Array<ParameterCreator>();
    public result: TypeOrCreator = TypeOrCreator.from(VoidType.REF);
    private attrs: HasAttributes
    private targetLinker: RuntimeLinker

    /** default constructor */
    public constructor() {
        this.attrs = new HasAttributes(new FreezableCreator(this), AllowedAttributes.LAMBDA)
        this.targetLinker = getNearestNonBootRuntimeLinker()!
    }

    /**
     * Forces creation of `this` creator and creators it depends on
     * @returns created Type instance
     */
    public override create(): LambdaType {
        return super.create() as LambdaType
    }

    /**
     * Appends parameter to `this` type
     * @param param parameter to add
     * @returns this
     */
    public addParameter(param: ParameterCreator): LambdaTypeCreator {
        this.checkNotCreated()
        param.frozen.freeze()
        if (param.name != undefined && param.name != '' + this.params.length) {
            throw new TypeAPICreateError("can't have such lambda parameter name")
        }
        this.params.push(param)
        return this;
    }

    /**
     * Sets result
     * @param typ type of result
     * @returns this
     */
    public addResult(typ: Type): LambdaTypeCreator {
        return this.addResult(TypeOrCreator.from(typ));
    }

    /**
     * Sets result
     * @param typ type of result
     * @returns this
     */
    public addResult(typ: TypeCreator): LambdaTypeCreator {
        return this.addResult(TypeOrCreator.from(typ));
    }

    /**
     * Sets result
     * @param typ type of result
     * @returns this
     */
    public addResult(typ: TypeOrCreator): LambdaTypeCreator {
        this.checkNotCreated()
        this.result = typ
        return this;
    }

    /**
     * Sets all provided attributes to `true`
     * @param attrs attribute to set
     * @returns this
     */
    public addAttributes(attrs: int): LambdaTypeCreator {
        this.attrs.add(attrs)
        return this;
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addAsync(): LambdaTypeCreator {
        this.attrs.add(Attributes.ASYNC, true)
        return this;
    }

    public override collectTypes(collector: CollectTypesCB): void {
        collector(this.result)
        for (let i = 0; i < this.params.length; i++) {
            collector(this.params.$_get(i).typ)
        }
    }

    public override declare(ctx: TypeCreatorCtx): void {
        if (this.ctxItemPtr !== 0) {
            throw new AssertionError("this.ctxItemPtr is not 0")
        }
        const ftPtr = TypeCreatorCtx.lambdaTypeCreate(ctx.ctxPtr, this.attrs.get())
        this.ctxItemPtr = ftPtr
        for (let i = 0; i < this.params.length; i++) {
            const par = this.params.$_get(i)
            ctx.checkError(TypeCreatorCtx.lambdaTypeAddParam(ftPtr, par.typ.getTypeDescriptor(), par.attrs.get()))
        }
        ctx.checkError(TypeCreatorCtx.lambdaTypeAddResult(ftPtr, this.result.getTypeDescriptor()))
        ctx.checkError(TypeCreatorCtx.lambdaTypeAdd(ftPtr))
    }

    public override define(ctx: TypeCreatorCtx): void {
    }

    protected override getTargetLinker(): RuntimeLinker {
        return this.targetLinker
    }
}

/**
 * interface that is used to freeze creators
 */
export interface Freezable {
    isFrozen(): boolean;
    freeze(): void;
}

/**
 * Checks if modificatiosn are allowed
 * @param freezable to check if modifications are allowed
 * @throws TypeAPICreateError if it is frozen
 */
function checkNotFrozen(freezable: Freezable): void {
    if (freezable.isFrozen()) {
        throw new TypeAPICreateError("modifying frozen entity")
    }
}

final class FreezableCreator extends TypeCreator implements Freezable {

    constructor(creator: TypeCreator) {
        super()
        this.typ = creator.typ
    }

    override isFrozen(): boolean {
        return this.typ != undefined
    }

    protected override getTargetLinker(): RuntimeLinker {
        return getNearestNonBootRuntimeLinker()!
    }

    public override freeze(): void {}

    public override collectTypes(collector: CollectTypesCB): void {}

    public override declare(ctx: TypeCreatorCtx): void {}

    public override define(ctx: TypeCreatorCtx): void {}
}

/**
 * Freezable used in features. It stores a boolean flag
 */
export final class FreezableImpl implements Freezable {
    private frozen: boolean = false

    override isFrozen(): boolean {
        return this.frozen
    }

    override freeze(): void {
        this.frozen = true
    }
}

/**
 * Storage for attributes
 */
export class HasAttributes {
    /** Freezable that prevents modification */
    private frozen: Freezable
    /** attributes */
    private attrs: int = 0
    /** attributes that can be modified */
    private readonly allowed: int

    constructor(frozen: Freezable, allowed: int) {
        this.frozen = frozen
        this.allowed = allowed
    }

    get(): int {
        return this.attrs;
    }

    has(attr: int): boolean {
        return (this.attrs & attr) != 0
    }

    /**
     * @enabled flag if attribute should be set to `true` or `false`
     */
    add(attr: int, enabled: boolean): void {
        if ((attr & this.allowed) != attr) {
            throw new TypeAPICreateError("disallowed attribute")
        }
        let newAttributes = this.attrs
        if (enabled) {
            newAttributes |= attr
        } else {
            newAttributes &= ~attr
        }
        if (this.attrs != newAttributes) {
            checkNotFrozen(this.frozen)
        }
        this.attrs = newAttributes
    }

    /**
     * @params attributes to be added
     */
    add(attrs: int): void {
        if ((attrs & this.allowed) != attrs) {
            throw new TypeAPICreateError("disallowed attributes")
        }
        if ((this.attrs & attrs) != attrs) {
            checkNotFrozen(this.frozen)
        }
        this.attrs = attrs
    }
}

/**
 * Storage for access modifier
 */
class HasAccessMod {
    frozen: Freezable
    accessMod: int = AccessModifier.PUBLIC

    constructor(frozen: Freezable) {
        this.frozen = frozen
    }

    get(): int {
        return this.accessMod;
    }

    set(accessMod: int): void {
        if (this.accessMod == accessMod) {
            return
        }
        checkNotFrozen(this.frozen)
        this.accessMod = accessMod
    }
}

/**
 * Creator for {@link Field}. Must be used as a part of {@link TypeCreator}.
 */
export final class FieldCreator {
    public name: string
    public typ: TypeOrCreator

    /**
     * @param name new field name
     * @param typ new field type
     */
    private constructor(name: string, typ: TypeOrCreator) {
        this.name = name
        this.attrs = new HasAttributes(this.frozen, AllowedAttributes.FIELD)
        this.accessMod = new HasAccessMod(this.frozen)
        this.typ = typ
    }

    private frozen: Freezable = new FreezableImpl()
    public attrs: HasAttributes
    private accessMod: HasAccessMod

    /**
     * @param name new field name
     * @param typ new field type
     */
    public constructor(name: string, typ: Type) {
        this(name, TypeOrCreator.from(typ))
    }

    public getAccessMod(): Int {
        return this.accessMod.get()
    }

    /**
     * @param name new field name
     * @param typ new field type
     */
    public constructor(name: string, typ: TypeCreator) {
        this(name, TypeOrCreator.from(typ))
    }

    /**
     * Sets all provided attributes to `true`
     * @param attrs attribute to set
     * @returns this
     */
    public addAttributes(attrs: int): FieldCreator {
        this.attrs.add(attrs)
        return this;
    }

    public getFrozen(): Freezable {
        return this.frozen
    }

    /**
     * Sets access modifier to provided
     * @param accessMod new access modifier
     * @returns this
     */
    public addAccessModifier(accessMod: int): FieldCreator {
        this.accessMod.set(accessMod);
        return this;
    }
}

/**
 * Creator for {@link Parameter}. Must be used as a part of {@link MethodCreator}.
 */
export final class ParameterCreator {
    public typ: TypeOrCreator
    public name?: string
    public frozen: FreezableImpl = new FreezableImpl()
    public attrs: HasAttributes

    /**
     * @param parameter type
     */
    public constructor(typ: Type) {
        this(TypeOrCreator.from(typ))
    }

    /**
     * @param parameter type
     * @param name name of parameter
     */
    public constructor(typ: Type, name: string) {
        this(TypeOrCreator.from(typ))
        this.name = name
    }

    /**
     * @param parameter type
     */
    public constructor(typ: TypeCreator) {
        this(TypeOrCreator.from(typ))
    }

    /**
     * @param parameter type
     */
    public constructor(typ: TypeOrCreator) {
        this.typ = typ
        this.attrs = new HasAttributes(this.frozen, AllowedAttributes.PARAMETER)
    }

    /**
     * @param name sets parameter name
     * @note by default name is index of parameter
     */
    public addName(name: string): ParameterCreator {
        this.name = name
        return this;
    }

    /**
     * Sets all provided attributes to `true`
     * @param attrs attribute to set
     * @returns this
     */
    public addAttributes(attrs: int): ParameterCreator {
        this.attrs.add(attrs)
        return this;
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addRest(): ParameterCreator {
        this.attrs.add(Attributes.REST, true)
        return this;
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addOptional(): ParameterCreator {
        this.attrs.add(Attributes.OPTIONAL, true)
        return this;
    }
}

/**
 * Creator for {@link Method}. Must be used as a part of {@link ClassTypeCreator} or {@link InterfaceTypeCreatpr}.
 */
export final class MethodCreator {
    public name: string
    public params: Array<ParameterCreator> = new Array<ParameterCreator>();
    public result: TypeOrCreator = TypeOrCreator.from(VoidType.REF);
    private body?: CallableBody
    public frozen: FreezableImpl = new FreezableImpl()
    public attrs: HasAttributes
    private accessMod: HasAccessMod

    /**
     * @param name method name, which can contain [a-zA-Z0-9] and be a valid identifier
     */
    public constructor(name: string) {
        this.name = name
        this.attrs = new HasAttributes(this.frozen, AllowedAttributes.METHOD)
        this.accessMod = new HasAccessMod(this.frozen)
    }

    /**
     * Adds body to method
     * @param body new body; undefined can be used in abstract methods
     */
    public addBody(body: CallableBody | undefined): MethodCreator {
        checkNotFrozen(this.frozen)
        this.body = body
        return this;
    }

    /**
     * Appends parameter to `this` method type
     * @param param parameter to add
     * @returns this
     */
    public addParameter(param: ParameterCreator): MethodCreator {
        checkNotFrozen(this.frozen)
        param.frozen.freeze()
        this.params.push(param)
        return this;
    }

    /**
     * Sets result
     * @param typ type of result
     * @returns this
     */
    public addResult(typ: Type): MethodCreator {
        return this.addResult(TypeOrCreator.from(typ))
    }

    /**
     * Sets result
     * @param typ type of result
     * @returns this
     */
    public addResult(typ: TypeCreator): MethodCreator {
        return this.addResult(TypeOrCreator.from(typ))
    }

    /**
     * Sets result
     * @param typ type of result
     * @returns this
     */
    public addResult(typ: TypeOrCreator): MethodCreator {
        checkNotFrozen(this.frozen)
        this.result = typ
        return this;
    }

    /**
     * Sets all provided attributes to `true`
     * @param attrs attribute to set
     * @returns this
     */
    public addAttributes(attrs: int): MethodCreator {
        this.attrs.add(attrs)
        return this;
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addConstructor(): MethodCreator {
        return this.addConstructor(true)
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addConstructor(isCtor: boolean): MethodCreator {
        this.attrs.add(Attributes.CONSTRUCTOR, isCtor);
        return this;
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addStatic(isStatic: boolean): MethodCreator {
        this.attrs.add(Attributes.STATIC, isStatic);
        return this;
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addGetter(): MethodCreator {
        this.attrs.add(Attributes.GETTER, true)
        return this;
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addSetter(): MethodCreator {
        this.attrs.add(Attributes.SETTER, true)
        return this;
    }

    /**
     * Adds attribute with same name
     * @returns this
     */
    public addAbstract(): MethodCreator {
        this.attrs.add(Attributes.ABSTRACT, true)
        return this;
    }

    /**
     * Sets access modifier to provided
     * @param accessMod new access modifier
     * @returns this
     */
    public addAccessModifier(accessMod: int): MethodCreator {
        this.accessMod.set(accessMod);
        return this;
    }

    /**
     * adds method to interface or class
     */
    public addToCtxItemPtr(ctx: TypeCreatorCtx, classOrIfacePtr: long): void {
        const fptr = ctx.checkCtxItemPtr(TypeCreatorCtx.methodCreate(classOrIfacePtr, this.name, this.attrs.get()))
        ctx.checkError(TypeCreatorCtx.methodAddAccessMod(fptr, this.accessMod.get()))
        for (let a = 0; a < this.params.length; a++) {
            const par = this.params.$_get(a)
            let name = par.name ?? '' + a;
            ctx.checkError(TypeCreatorCtx.methodAddParam(fptr, par.typ.getTypeDescriptor(), name, par.attrs.get()))
        }
        ctx.checkError(TypeCreatorCtx.methodAddResult(fptr, this.result.getTypeDescriptor()))
        const isAbstract = this.attrs.has(Attributes.ABSTRACT)
        const hasBody = this.body != undefined
        if (isAbstract == hasBody) {
            ctx.checkError("only and only abstract methods don't have a body; abstract = " + isAbstract + "; body = " + (this.body == undefined ? "undefined" : this.body!.toString()))
        }
        if (!isAbstract) {
            const body = this.body!
            if (body instanceof CallableBodyMethod) {
                const callableBody = (body as CallableBodyMethod).getBody()
                ctx.checkError(TypeCreatorCtx.methodAddBodyFromMethod(fptr, callableBody.getType()))
            } else if (body instanceof CallableBodyFunction) {
                const bbody = (body as CallableBodyFunction).getBody()
                const id = ctx.registerObject(bbody as Object);
                const bbodyType = Type.of(bbody) as LambdaType
                ctx.checkError(TypeCreatorCtx.methodAddBodyFromLambda(fptr, id, bbodyType))
            } else if (body instanceof CallableBodyErasedFunction) {
                const bbody = (body as CallableBodyErasedFunction).getBody()
                const id = ctx.registerObject(bbody as Object);
                ctx.checkError(TypeCreatorCtx.methodAddBodyFromErasedLambda(fptr, id))
            } else if (body instanceof CallableBodyDefault) {
                ctx.checkError(TypeCreatorCtx.methodAddBodyDefault(fptr))
            } else {
                throw new TypeAPICreateError("body not supported" + this.body)
            }
        }
        ctx.checkError(TypeCreatorCtx.methodAdd(fptr))
    }
}

/**
 * Type that abstracts all supported function/lambda/method body-types
 */
export class CallableBody {
    /** Makes class package-sealed */
    protected constructor() {}
}

/**
 * Represents default body: does nothing for regular constructors (returns default value).
 */
export final class CallableBodyDefault extends CallableBody {
    public static readonly INSTANCE = new CallableBodyDefault()

    public constructor() {}
}

/**
 * Represents call to other method with forwarding all arguments as-is
 */
export final class CallableBodyMethod extends CallableBody {
    private readonly body: Method

    public constructor(m: Method) {
        super()
        this.body = m
    }
    public getBody(): Method {
        return this.body
    }
}

/**
 * Represents call to lambda with forwarding all arguments as-is
 */
export final class CallableBodyFunction extends CallableBody {
    private readonly body: Object

    /**
     * @param m lambda to call
     * @example new CallableBodyFunction((self: Object, i: int): void {} as Object)
     * @note in case of instance methods zero argument is a reciever
     */
    public constructor(m: Object) {
        super()
        if (!(Type.of(m) instanceof LambdaType)) {
            throw new TypeAPICreateError("can't encapsulate non-functional type")
        }
        this.body = m
    }
    public getBody(): Object {
        return this.body
    }
}

/**
 * Represents call to lambda with boxing all arguments
 */
export final class CallableBodyErasedFunction extends CallableBody {
    private /* readonly */ body: (recv: Any, args: FixedArray<Any>) => Any
    public constructor(m: (recv: Any, args: FixedArray<Any>) => Any) {
        super()
        this.body = m
    }

    public getBody(): (recv: Any, args: FixedArray<Any>) => Any {
        return this.body
    }
}
