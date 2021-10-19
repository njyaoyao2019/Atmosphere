/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <mesosphere/kern_common.hpp>
#include <mesosphere/kern_k_typed_address.hpp>
#include <mesosphere/kern_k_class_token.hpp>

namespace ams::kern {

    class KProcess;

    #define MESOSPHERE_AUTOOBJECT_TRAITS(CLASS, BASE_CLASS)                                                     \
        NON_COPYABLE(CLASS);                                                                                    \
        NON_MOVEABLE(CLASS);                                                                                    \
        private:                                                                                                \
            friend class ::ams::kern::KClassTokenGenerator;                                                     \
            static constexpr inline auto ObjectType = ::ams::kern::KClassTokenGenerator::ObjectType::CLASS;     \
            static constexpr inline const char * const TypeName = #CLASS;                                       \
            static constexpr inline ClassTokenType ClassToken() { return ::ams::kern::ClassToken<CLASS>; }      \
        public:                                                                                                 \
            using BaseClass = BASE_CLASS;                                                                       \
            static constexpr ALWAYS_INLINE TypeObj GetStaticTypeObj() {                                         \
                constexpr ClassTokenType Token = ClassToken();                                                  \
                return TypeObj(TypeName, Token);                                                                \
            }                                                                                                   \
            static constexpr ALWAYS_INLINE const char *GetStaticTypeName() { return TypeName; }                 \
            virtual TypeObj GetTypeObj() const { return GetStaticTypeObj(); }                                   \
            virtual const char *GetTypeName() { return GetStaticTypeName(); }                                   \
        private:

    class KAutoObject {
        public:
            class ReferenceCount {
                NON_COPYABLE(ReferenceCount);
                NON_MOVEABLE(ReferenceCount);
                private:
                    using Storage = u32;
                private:
                    util::Atomic<Storage> m_value;
                public:
                    ALWAYS_INLINE explicit ReferenceCount() { /* ... */ }
                    constexpr ALWAYS_INLINE explicit ReferenceCount(Storage v) : m_value(v) { /* ... */ }

                    ALWAYS_INLINE void operator=(Storage v) { m_value = v; }

                    ALWAYS_INLINE Storage GetValue() const { return m_value.Load(); }

                    ALWAYS_INLINE bool Open() {
                        /* Atomically increment the reference count, only if it's positive. */
                        u32 cur = m_value.Load<std::memory_order_relaxed>();
                        do {
                            if (AMS_UNLIKELY(cur == 0)) {
                                MESOSPHERE_AUDIT(cur != 0);
                                return false;
                            }
                            MESOSPHERE_ABORT_UNLESS(cur < cur + 1);
                        } while (AMS_UNLIKELY(!m_value.CompareExchangeWeak<std::memory_order_relaxed>(cur, cur + 1)));

                        return true;
                    }

                    ALWAYS_INLINE bool Close() {
                        /* Atomically decrement the reference count, not allowing it to become negative. */
                        u32 cur = m_value.Load<std::memory_order_relaxed>();
                        do {
                            MESOSPHERE_ABORT_UNLESS(cur > 0);
                        } while (AMS_UNLIKELY(!m_value.CompareExchangeWeak<std::memory_order_relaxed>(cur, cur - 1)));

                        /* Return whether the object was closed. */
                        return cur - 1 == 0;
                    }
            };
        protected:
            class TypeObj {
                private:
                    const char *m_name;
                    ClassTokenType m_class_token;
                public:
                    constexpr explicit TypeObj(const char *n, ClassTokenType tok) : m_name(n), m_class_token(tok) { /* ... */ }

                    constexpr ALWAYS_INLINE const char *GetName() const { return m_name; }
                    constexpr ALWAYS_INLINE ClassTokenType GetClassToken() const { return m_class_token; }

                    constexpr ALWAYS_INLINE bool operator==(const TypeObj &rhs) {
                        return this->GetClassToken() == rhs.GetClassToken();
                    }

                    constexpr ALWAYS_INLINE bool operator!=(const TypeObj &rhs) {
                        return this->GetClassToken() != rhs.GetClassToken();
                    }

                    constexpr ALWAYS_INLINE bool IsDerivedFrom(const TypeObj &rhs) {
                        return IsClassTokenDerivedFrom(this->GetClassToken(), rhs.GetClassToken());
                    }

                    static constexpr ALWAYS_INLINE bool IsClassTokenDerivedFrom(ClassTokenType derived, ClassTokenType base) {
                        return (derived | base) == derived;
                    }
            };
        private:
            MESOSPHERE_AUTOOBJECT_TRAITS(KAutoObject, KAutoObject);
        private:
            KAutoObject *m_next_closed_object;
            ReferenceCount m_ref_count;
            #if defined(MESOSPHERE_ENABLE_DEVIRTUALIZED_DYNAMIC_CAST)
            ClassTokenType m_class_token;
            #endif
        public:
            static KAutoObject *Create(KAutoObject *ptr);
        public:
            constexpr ALWAYS_INLINE explicit KAutoObject() : m_next_closed_object(nullptr), m_ref_count(0)
            #if defined(MESOSPHERE_ENABLE_DEVIRTUALIZED_DYNAMIC_CAST)
                , m_class_token(0)
            #endif
            {
                MESOSPHERE_ASSERT_THIS();
            }

            /* Destroy is responsible for destroying the auto object's resources when ref_count hits zero. */
            virtual void Destroy() { MESOSPHERE_ASSERT_THIS(); }

            /* Finalize is responsible for cleaning up resource, but does not destroy the object. */
            virtual void Finalize() { MESOSPHERE_ASSERT_THIS(); }

            virtual KProcess *GetOwner() const { return nullptr; }

            u32 GetReferenceCount() const {
                return m_ref_count.GetValue();
            }

            ALWAYS_INLINE bool IsDerivedFrom(const TypeObj &rhs) const {
                #if defined(MESOSPHERE_ENABLE_DEVIRTUALIZED_DYNAMIC_CAST)
                    return TypeObj::IsClassTokenDerivedFrom(m_class_token, rhs.GetClassToken());
                #else
                    return this->GetTypeObj().IsDerivedFrom(rhs);
                #endif
            }

            ALWAYS_INLINE bool IsDerivedFrom(const KAutoObject &rhs) const {
                #if defined(MESOSPHERE_ENABLE_DEVIRTUALIZED_DYNAMIC_CAST)
                    return TypeObj::IsClassTokenDerivedFrom(m_class_token, rhs.m_class_token);
                #else
                    return this->IsDerivedFrom(rhs.GetTypeObj());
                #endif
            }

            template<typename Derived>
            ALWAYS_INLINE Derived DynamicCast() {
                static_assert(std::is_pointer<Derived>::value);
                using DerivedType = typename std::remove_pointer<Derived>::type;

                if (AMS_LIKELY(this->IsDerivedFrom(DerivedType::GetStaticTypeObj()))) {
                    return static_cast<Derived>(this);
                } else {
                    return nullptr;
                }
            }

            template<typename Derived>
            ALWAYS_INLINE const Derived DynamicCast() const {
                static_assert(std::is_pointer<Derived>::value);
                using DerivedType = typename std::remove_pointer<Derived>::type;

                if (AMS_LIKELY(this->IsDerivedFrom(DerivedType::GetStaticTypeObj()))) {
                    return static_cast<Derived>(this);
                } else {
                    return nullptr;
                }
            }

            NOINLINE bool Open() {
                MESOSPHERE_ASSERT_THIS();

                return m_ref_count.Open();
            }

            NOINLINE void Close() {
                MESOSPHERE_ASSERT_THIS();

                if (m_ref_count.Close()) {
                    this->ScheduleDestruction();
                }
            }
        private:
            /* NOTE: This has to be defined *after* KThread is defined. */
            /* Nintendo seems to handle this by defining Open/Close() in a cpp, but we'd like them to remain in headers. */
            /* Implementation for this will be inside kern_k_thread.hpp, so it can be ALWAYS_INLINE. */
            void ScheduleDestruction();
        public:
            /* Getter, for KThread. */
            ALWAYS_INLINE KAutoObject *GetNextClosedObject() { return m_next_closed_object; }
    };

    class KAutoObjectWithListContainer;

    class KAutoObjectWithListBase : public KAutoObject {
        private:
            void *m_alignment_forcer_unused[0]{};
        public:
            constexpr ALWAYS_INLINE KAutoObjectWithListBase() = default;
    };

    class KAutoObjectWithList : public KAutoObjectWithListBase {
        private:
            friend class KAutoObjectWithListContainer;
        private:
            util::IntrusiveRedBlackTreeNode m_list_node;
        public:
            constexpr ALWAYS_INLINE KAutoObjectWithList() : m_list_node() { /* ... */ }

            static ALWAYS_INLINE int Compare(const KAutoObjectWithList &lhs, const KAutoObjectWithList &rhs) {
                const u64 lid = lhs.GetId();
                const u64 rid = rhs.GetId();

                if (lid < rid) {
                    return -1;
                } else if (lid > rid) {
                    return 1;
                } else {
                    return 0;
                }
            }
        public:
            virtual u64 GetId() const {
                return reinterpret_cast<u64>(this);
            }
    };

    template<typename T> requires std::derived_from<T, KAutoObject>
    class KScopedAutoObject {
        NON_COPYABLE(KScopedAutoObject);
        private:
            template<typename U>
            friend class KScopedAutoObject;
        private:
            T *m_obj;
        private:
            constexpr ALWAYS_INLINE void Swap(KScopedAutoObject &rhs) {
                std::swap(m_obj, rhs.m_obj);
            }
        public:
            constexpr ALWAYS_INLINE KScopedAutoObject() : m_obj(nullptr) { /* ... */ }
            constexpr ALWAYS_INLINE KScopedAutoObject(T *o) : m_obj(o) {
                if (m_obj != nullptr) {
                    m_obj->Open();
                }
            }

            ALWAYS_INLINE ~KScopedAutoObject() {
                if (m_obj != nullptr) {
                    m_obj->Close();
                }
                m_obj = nullptr;
            }

            template<typename U> requires (std::derived_from<T, U> || std::derived_from<U, T>)
            constexpr KScopedAutoObject(KScopedAutoObject<U> &&rhs) {
                if constexpr (std::derived_from<U, T>) {
                    /* Upcast. */
                    m_obj = rhs.m_obj;
                    rhs.m_obj = nullptr;
                } else {
                    /* Downcast. */
                    T *derived = nullptr;
                    if (rhs.m_obj != nullptr) {
                        derived = rhs.m_obj->template DynamicCast<T *>();
                        if (derived == nullptr) {
                            rhs.m_obj->Close();
                        }
                    }

                    m_obj = derived;
                    rhs.m_obj = nullptr;
                }
            }

            constexpr ALWAYS_INLINE KScopedAutoObject<T> &operator=(KScopedAutoObject<T> &&rhs) {
                rhs.Swap(*this);
                return *this;
            }

            constexpr ALWAYS_INLINE T *operator->() { return m_obj; }
            constexpr ALWAYS_INLINE T &operator*() { return *m_obj; }

            constexpr ALWAYS_INLINE void Reset(T *o) {
                KScopedAutoObject(o).Swap(*this);
            }

            constexpr ALWAYS_INLINE T *GetPointerUnsafe() { return m_obj; }

            constexpr ALWAYS_INLINE T *ReleasePointerUnsafe() { T *ret = m_obj; m_obj = nullptr; return ret; }

            constexpr ALWAYS_INLINE bool IsNull() const { return m_obj == nullptr; }
            constexpr ALWAYS_INLINE bool IsNotNull() const { return m_obj != nullptr; }
    };

    template<typename T> requires std::derived_from<T, KAutoObject>
    class KSharedAutoObject {
        private:
            T *m_object;
            KAutoObject::ReferenceCount m_ref_count;
        public:
            explicit KSharedAutoObject() : m_object(nullptr) { /* ... */ }

            void Attach(T *obj) {
                MESOSPHERE_ASSERT(m_object == nullptr);

                /* Set our object. */
                m_object = obj;

                /* Open reference to our object. */
                m_object->Open();

                /* Set our reference count. */
                m_ref_count = 1;
            }

            bool Open() {
                return m_ref_count.Open();
            }

            void Close() {
                if (m_ref_count.Close()) {
                    this->Detach();
                }
            }

            ALWAYS_INLINE T *Get() const {
                return m_object;
            }
        private:
            void Detach() {
                /* Close our object, if we have one. */
                if (T * const object = m_object; AMS_LIKELY(object != nullptr)) {
                    /* Set our object to a debug sentinel value, which will cause crash if accessed. */
                    m_object = reinterpret_cast<T *>(1);

                    /* Close reference to our object. */
                    object->Close();
                }
            }
    };


}
